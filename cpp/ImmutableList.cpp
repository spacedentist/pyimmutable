#include "ImmutableList.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

#include "Hash.h"
#include "PyObjectRef.h"
#include "util.h"

namespace pyimmutable {

namespace {

struct ListItem {
  PyObjectRef value;
  Sha1Hash valueHash;
  Sha1Hash itemHash;
  bool isImmutableJson;
};

using VectorType = immer::vector<ListItem>;
using TransientVectorType = immer::vector_transient<ListItem>;

struct ImmutableListIter {
  PyObject_HEAD;

  struct State {
    VectorType::const_iterator iter;
    VectorType::const_iterator end;
    PyObjectRef immutableList;
  };
  union {
    State state;
  };

  PyObject* next() {
    if (state.iter == state.end) {
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
    return state.iter++->value.copy().release();
  }

  static void destroy(PyObject* pyself) {
    ImmutableListIter* const self =
        reinterpret_cast<ImmutableListIter*>(pyself);

    std::addressof(self->state)->~State();
    PyObject_Del(pyself);
  }
};

struct ImmutableList {
  PyObject_HEAD;

  struct State {
    VectorType const vec;
    Sha1Hash const sha1;
    std::size_t immutableJsonItems;
    bool isImmutableJson;

    State(VectorType&& vecx, Sha1Hash sha1, std::size_t immutable_json_items)
        : vec(std::move(vecx)),
          sha1(sha1),
          immutableJsonItems(immutable_json_items),
          isImmutableJson(immutableJsonItems == vec.size()) {}
  };
  union {
    State state;
  };
  bool hasState{false};

  static std::unordered_map<Sha1Hash, ImmutableList*, Sha1HashHasher> lookUpMap;

  PyObject* getItemIdx(Py_ssize_t idx) noexcept {
    Py_ssize_t const len = state.vec.size();

    if (len == 0 || idx >= len || idx < -len) {
      PyErr_SetNone(PyExc_IndexError);
      return nullptr;
    } else {
      if (idx < 0) {
        idx = len + idx;
      }
      return state.vec[idx].value.copy().release();
    }
  }

  PyObject* getItem(PyObject* item) noexcept {
    if (PyIndex_Check(item)) {
      PyObjectRef idx{PyNumber_Index(item), false};
      if (!idx) {
        return nullptr;
      }
      return getItemIdx(PyLong_AsSsize_t(idx.get()));
    }
    if (PySlice_Check(item)) {
      Py_ssize_t start, stop, step;

      if (PySlice_Unpack(item, &start, &stop, &step) != 0) {
        return nullptr;
      }
      return makeSlice(start, stop, step).release();
    }

    PyErr_Format(
        PyExc_TypeError,
        "range indices must be integers or slices, not %.200s",
        item->ob_type->tp_name);
    return nullptr;
  }

  TypedPyObjectRef<ImmutableList>
  makeSlice(Py_ssize_t start, Py_ssize_t stop, Py_ssize_t step) {
    if (step == 0) {
      return {};
    }

    Py_ssize_t length =
        PySlice_AdjustIndices(state.vec.size(), &start, &stop, step);

    if (length <= 0) {
      return makeEmpty();
    }

    if (start == 0 && step == 1) {
      if (length >= static_cast<Py_ssize_t>(state.vec.size())) {
        return TypedPyObjectRef{this};
      }

      Sha1Hash hash{0};
      std::size_t immutable_json_items = 0;
      for (auto it = state.vec.begin(); start < stop; ++start, ++it) {
        xorHashInPlace(hash, it->itemHash);
        if (it->isImmutableJson) {
          ++immutable_json_items;
        }
      }

      return getOrCreate(
          hash, [&]() { return state.vec.take(length); }, immutable_json_items);
    }

    Sha1Hash hash{0};
    std::size_t immutable_json_items = 0;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(length);

    for (Py_ssize_t i = 0; i < length; ++i) {
      auto const& src_item = state.vec[start + i * step];
      auto item_hash = itemHash(src_item.valueHash, i);
      item_hashes.push_back(item_hash);
      xorHashInPlace(hash, item_hash);
      if (src_item.isImmutableJson) {
        ++immutable_json_items;
      }
    }

    return getOrCreate(
        hash,
        [&]() {
          TransientVectorType tvec;
          for (Py_ssize_t i = 0; i < length; ++i) {
            auto const& src_item = state.vec[start + i * step];
            auto const& item_hash = item_hashes[i];
            tvec.push_back(ListItem{src_item.value,
                                    src_item.valueHash,
                                    item_hash,
                                    src_item.isImmutableJson});
          }

          return std::move(tvec).persistent();
        },
        immutable_json_items);
  }

  PyObject* set(PyObject* args) noexcept {
    Py_ssize_t idx = 0;
    PyObject* value = nullptr;

    if (!PyArg_ParseTuple(args, "nO", &idx, &value)) {
      return NULL;
    }

    if (idx < 0) {
      idx += state.vec.size();
    }
    if (idx < 0 or idx >= static_cast<Py_ssize_t>(state.vec.size())) {
      PyErr_SetNone(PyExc_IndexError);
      return nullptr;
    }

    auto const& src_item = state.vec[idx];
    auto const hvalue = valueHash(value);

    if (hvalue == src_item.valueHash) {
      return TypedPyObjectRef{this}.release();
    }

    auto const hitem = itemHash(hvalue, idx);
    auto const vec_hash =
        xorHash(xorHash(state.sha1, src_item.itemHash), hitem);
    bool const is_immutable_json = isImmutableJsonObject(value);
    auto const immutable_json_items = state.immutableJsonItems -
        (src_item.isImmutableJson ? 1 : 0) + (is_immutable_json ? 1 : 0);

    return getOrCreate(
               vec_hash,
               [&]() {
                 return state.vec.set(
                     idx,
                     ListItem{PyObjectRef{value},
                              hvalue,
                              hitem,
                              immutable_json_items});
               },
               immutable_json_items)
        .release();
  }

  PyObject* append(PyObject* value) noexcept {
    auto const hvalue = valueHash(value);
    auto const hitem = itemHash(hvalue, state.vec.size());
    auto vec_hash = xorHash(state.sha1, hitem);
    bool const is_immutable_json = isImmutableJsonObject(value);
    auto const immutable_json_items =
        state.immutableJsonItems + (is_immutable_json ? 1 : 0);

    return getOrCreate(
               vec_hash,
               [&]() {
                 return state.vec.push_back(ListItem{
                     PyObjectRef{value}, hvalue, hitem, is_immutable_json});
               },
               immutable_json_items)
        .release();
  }

  int contains(PyObject* value) noexcept {
    auto const hvalue = valueHash(value);

    for (auto const& item : state.vec) {
      if (item.valueHash == hvalue) {
        return 1;
      }
    }

    return 0;
  }

  Py_ssize_t len() {
    return state.vec.size();
  }

  PyObject* concat(PyObject* rhs_ptr) {
    if (Py_TYPE(rhs_ptr) != &ImmutableList_typeObject) {
      PyErr_Format(
          PyExc_TypeError,
          "can only concatenate ImmutableList "
          "(not \"%.200s\") to ImmutableList",
          rhs_ptr->ob_type->tp_name);
      return nullptr;
    }

    ImmutableList& rhs = *reinterpret_cast<ImmutableList*>(rhs_ptr);

    if (rhs.state.vec.empty()) {
      return TypedPyObjectRef{this}.release();
    }

    if (state.vec.empty()) {
      return TypedPyObjectRef{&rhs}.release();
    }

    Py_ssize_t idx = state.vec.size();
    Sha1Hash hash = state.sha1;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(rhs.state.vec.size());

    for (auto const& item : rhs.state.vec) {
      auto hitem = itemHash(item.valueHash, idx++);
      item_hashes.push_back(hitem);
      xorHashInPlace(hash, hitem);
    }

    return getOrCreate(
               hash,
               [&]() {
                 auto tvec = state.vec.transient();

                 auto it = item_hashes.begin();
                 for (auto const& item : rhs.state.vec) {
                   tvec.push_back(ListItem{item.value,
                                           item.valueHash,
                                           *it++,
                                           item.isImmutableJson});
                 }

                 return std::move(tvec).persistent();
               },
               state.immutableJsonItems + rhs.state.immutableJsonItems)
        .release();
  }

  PyObject* repeat(Py_ssize_t count) {
    if (count <= 0 or state.vec.empty()) {
      return makeEmpty().release();
    }
    if (count == 1) {
      return TypedPyObjectRef{this}.release();
    }

    Py_ssize_t idx = state.vec.size();
    Sha1Hash hash = state.sha1;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(idx * (count - 1));

    for (Py_ssize_t c = 1; c < count; ++c) {
      for (auto const& item : state.vec) {
        auto hitem = itemHash(item.valueHash, idx++);
        item_hashes.push_back(hitem);
        xorHashInPlace(hash, hitem);
      }
    }

    return getOrCreate(
               hash,
               [&]() {
                 auto tvec = state.vec.transient();

                 auto it = item_hashes.begin();
                 for (Py_ssize_t c = 1; c < count; ++c) {
                   for (auto const& item : state.vec) {
                     tvec.push_back(ListItem{item.value,
                                             item.valueHash,
                                             *it++,
                                             item.isImmutableJson});
                   }
                 }

                 return std::move(tvec).persistent();
               },
               state.immutableJsonItems * count)
        .release();
  }

  PyObject* iter() {
    auto iter = TypedPyObjectRef<ImmutableListIter>::create(
        &ImmutableListIter_typeObject);

    if (iter) {
      new (std::addressof(iter->state)) ImmutableListIter::State();

      iter->state.iter = state.vec.begin();
      iter->state.end = state.vec.end();
      iter->state.immutableList = TypedPyObjectRef{this};
    }

    return iter.release();
  }

  PyObject* repr() {
    PyObjectRef result{PyUnicode_FromString("ImmutableList(["), false};
    PyObjectRef item_sep;

    if (!result) {
      return nullptr;
    }

    Py_ReprEnter(reinterpret_cast<PyObject*>(this));
    OnDestroy repr_leave{
        [this]() { Py_ReprLeave(reinterpret_cast<PyObject*>(this)); }};

    bool first = true;
    for (auto const& item : state.vec) {
      PyObjectRef value{PyObject_Repr(item.value.get()), false};
      if (!value) {
        return nullptr;
      }

      if (!first) {
        if (!item_sep) {
          item_sep = PyObjectRef{PyUnicode_FromString(", "), false};
          if (!item_sep) {
            return nullptr;
          }
        }

        result =
            PyObjectRef{PyUnicode_Concat(result.get(), item_sep.get()), false};
        if (!result) {
          return nullptr;
        }
      }

      result = PyObjectRef{PyUnicode_Concat(result.get(), value.get()), false};
      if (!result) {
        return nullptr;
      }

      first = false;
    }

    PyObjectRef end{PyUnicode_FromString("])"), false};
    if (!end) {
      return nullptr;
    }
    result = PyObjectRef{PyUnicode_Concat(result.get(), end.get()), false};

    return result.release();
  }

  PyObject* isImmutableJsonList(void* /* unused */) {
    return PyObjectRef(state.isImmutableJson ? Py_True : Py_False).release();
  }

  template <typename F>
  static TypedPyObjectRef<ImmutableList> getOrCreate(
      Sha1Hash hash,
      F&& vec_factory,
      std::size_t immutable_json_items) {
    auto it = lookUpMap.find(hash);
    if (it != lookUpMap.end()) {
      return TypedPyObjectRef{it->second};
    }

    auto obj =
        TypedPyObjectRef<ImmutableList>::create(&ImmutableList_typeObject);

    if (obj) {
      new (std::addressof(obj->state))
          State(std::forward<F>(vec_factory)(), hash, immutable_json_items);
      obj->hasState = true;

      try {
        lookUpMap.emplace(hash, obj.get());
      } catch (...) {
        obj->state.~State();
        obj->hasState = false;
        obj = {};
      }
    }

    return obj;
  }

  static TypedPyObjectRef<ImmutableList> makeEmpty() {
    return getOrCreate({}, []() { return VectorType{}; }, 0);
  }

  static PyObject* new_(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    if (!_PyArg_NoKeywords("ImmutableList", kwds)) {
      return nullptr;
    }

    PyObject* arg = nullptr;
    if (!PyArg_UnpackTuple(args, "ImmutableList", 0, 1, &arg)) {
      return nullptr;
    }

    if (arg) {
      Sha1Hash vec_hash{0};
      std::size_t immutable_json_items = 0;
      TransientVectorType tvec;
      if (!extendCommon(vec_hash, immutable_json_items, tvec, arg)) {
        return nullptr;
      }
      return getOrCreate(
                 vec_hash,
                 [&]() { return std::move(tvec).persistent(); },
                 immutable_json_items)
          .release();
    } else {
      return makeEmpty().release();
    }
  }

  PyObject* extend(PyObject* seq) noexcept {
    if (Py_TYPE(seq) == &ImmutableList_typeObject) {
      // optimisation: if argument is ImmutableList we can use concat,
      // which does not have to compute value hashes
      return concat(seq);
    }

    auto vec_hash = state.sha1;
    auto immutable_json_items = state.immutableJsonItems;
    auto tvec = state.vec.transient();
    if (!extendCommon(vec_hash, immutable_json_items, tvec, seq)) {
      return nullptr;
    }
    return getOrCreate(
               vec_hash,
               [&]() { return std::move(tvec).persistent(); },
               immutable_json_items)
        .release();
  }

  static bool extendCommon(
      Sha1Hash& hash,
      std::size_t& immutable_json_items,
      TransientVectorType& tvec,
      PyObject* arg) {
    PyObjectRef iter{PyObject_GetIter(arg), false};
    if (!iter) {
      return false;
    }

    while (auto value = PyObjectRef{PyIter_Next(iter.get()), false}) {
      auto const hvalue = valueHash(value.get());
      auto const hitem = itemHash(hvalue, tvec.size());
      xorHashInPlace(hash, hitem);
      auto is_immutable_json = isImmutableJsonObject(value.get());
      if (is_immutable_json) {
        ++immutable_json_items;
      }
      tvec.push_back(
          ListItem{std::move(value), hvalue, hitem, is_immutable_json});
    }

    return !PyErr_Occurred();
  }

  static void destroy(PyObject* pyself) {
    ImmutableList* const self = reinterpret_cast<ImmutableList*>(pyself);

    if (self->hasState) {
      lookUpMap.erase(self->state.sha1);
      self->state.~State();
    }

    PyObject_Del(pyself);
  }

  static PyObject* getInstanceCount(PyObject*, PyObject*) {
    return PyLong_FromSize_t(lookUpMap.size());
  }
};

std::unordered_map<Sha1Hash, ImmutableList*, Sha1HashHasher>
    ImmutableList::lookUpMap{};

PyMethodDef ImmutableList_methods[] = {
    {"set",
     method<ImmutableList, &ImmutableList::set>(),
     METH_VARARGS,
     "docstring"},
    {"append",
     method<ImmutableList, &ImmutableList::append>(),
     METH_O,
     "docstring"},
    {"extend",
     method<ImmutableList, &ImmutableList::extend>(),
     METH_O,
     "docstring"},
    {"_get_instance_count",
     &ImmutableList::getInstanceCount,
     METH_NOARGS | METH_STATIC,
     "docstring"},
    {nullptr}};

PySequenceMethods ImmutableList_sequenceMethods = {
    .sq_length = method<ImmutableList, &ImmutableList::len>(),
    .sq_concat = method<ImmutableList, &ImmutableList::concat>(),
    .sq_repeat = method<ImmutableList, &ImmutableList::repeat>(),
    .sq_item = method<ImmutableList, &ImmutableList::getItemIdx>(),
    .sq_contains = method<ImmutableList, &ImmutableList::contains>(),
};

PyMappingMethods ImmutableList_mappingMethods = {
    .mp_length = method<ImmutableList, &ImmutableList::len>(),
    .mp_subscript = method<ImmutableList, &ImmutableList::getItem>(),
    .mp_ass_subscript = nullptr,
};

PyGetSetDef ImmutableList_getset[] = {
    {"isImmutableJson",
     method<ImmutableList, &ImmutableList::isImmutableJsonList>(),
     nullptr,
     "docstring",
     nullptr},
    {nullptr}};

} // namespace

bool isImmutableJsonList(PyObject* obj) {
  return reinterpret_cast<ImmutableList*>(obj)->state.isImmutableJson;
}

PyTypeObject ImmutableList_typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableList",
    .tp_basicsize = sizeof(ImmutableList),
    .tp_dealloc = &ImmutableList::destroy,
    .tp_repr = method<ImmutableList, &ImmutableList::repr>(),
    .tp_as_sequence = &ImmutableList_sequenceMethods,
    .tp_as_mapping = &ImmutableList_mappingMethods,
    .tp_doc = "(to be written)",
    .tp_iter = method<ImmutableList, &ImmutableList::iter>(),
    .tp_methods = ImmutableList_methods,
    .tp_getset = ImmutableList_getset,
    .tp_new = &ImmutableList::new_,
};
PyTypeObject ImmutableListIter_typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableListIterator",
    .tp_basicsize = sizeof(ImmutableListIter),
    .tp_dealloc = &ImmutableListIter::destroy,
    .tp_iter = [](PyObject* self) { return PyObjectRef{self}.release(); },
    .tp_iternext = method<ImmutableListIter, &ImmutableListIter::next>(),
};

} // namespace pyimmutable
