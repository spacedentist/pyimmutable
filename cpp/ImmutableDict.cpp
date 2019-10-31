/*
 * MIT License
 *
 * Copyright (c) Sven Over <sp@cedenti.st>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ImmutableDict.h"

#include <cstddef>
#include <memory>
#include <unordered_map>

#include <immer/map.hpp>

#include "Hash.h"
#include "PyObjectRef.h"
#include "util.h"

namespace pyimmutable {

namespace {

struct DictItem {
  PyObjectRef key;
  PyObjectRef value;
  Sha1Hash valueHash;
  bool isImmutableJson;
};

bool isImmutableJsonItem(PyObject* key, PyObject* value) {
  return PyUnicode_CheckExact(key) && isImmutableJsonObject(value);
}

using MapType = immer::map<Sha1Hash, DictItem, Sha1HashHasher>;

struct ImmutableDictIter {
  using Extractor = PyObjectRef (*)(DictItem const&);

  PyObject_HEAD;

  struct State {
    MapType::const_iterator iter;
    MapType::const_iterator end;
    PyObjectRef immutableDict;
    Extractor extractor;
  };
  union {
    State state;
  };

  PyObject* next() {
    if (state.iter == state.end) {
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
    return state.extractor(state.iter++->second).copy().release();
  }

  static PyObjectRef keyExtractor(DictItem const& item) {
    return item.key;
  }

  static PyObjectRef valueExtractor(DictItem const& item) {
    return item.value;
  }

  static PyObjectRef itemExtractor(DictItem const& item) {
    return buildValue("OO", item.key.get(), item.value.get());
  }

  static void destroy(PyObject* pyself) {
    ImmutableDictIter* const self =
        reinterpret_cast<ImmutableDictIter*>(pyself);

    std::addressof(self->state)->~State();
    PyObject_Del(pyself);
  }
};

struct ImmutableDict {
  PyObject_HEAD;
  PyObject* weakreflist; // List of weak references
  bool hasState;

  struct State {
    MapType const map;
    Sha1Hash const sha1;
    std::size_t immutableJsonItems;
    bool isImmutableJson;
    PyObjectRef meta;

    State(MapType&& mapx, Sha1Hash sha1, std::size_t immutable_json_items)
        : map(std::move(mapx)),
          sha1(sha1),
          immutableJsonItems(immutable_json_items),
          isImmutableJson(immutableJsonItems == map.size()) {}
  };
  union {
    State state;
  };

  static std::unordered_map<Sha1Hash, ImmutableDict*, Sha1HashHasher> lookUpMap;

  PyObject* getItem(PyObject* key) noexcept {
    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = state.map.find(h);
    if (!ptr) {
      PyErr_SetObject(PyExc_KeyError, PyObjectRef{key}.release());
      return nullptr;
    } else {
      return ptr->value.copy().release();
    }
  }

  PyObject* get(PyObject* args) noexcept {
    PyObject* key = nullptr;
    PyObject* default_value = Py_None;

    if (!PyArg_ParseTuple(args, "O|O", &key, &default_value)) {
      return NULL;
    }

    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = state.map.find(h);
    if (!ptr) {
      return PyObjectRef{default_value}.release();
    } else {
      return ptr->value.copy().release();
    }
  }

  PyObject* set(PyObject* args) noexcept {
    PyObject* key = nullptr;
    PyObject* value = nullptr;

    if (!PyArg_ParseTuple(args, "OO", &key, &value)) {
      return NULL;
    }

    auto const [hkey, hvalue] = keyValueHashes(key, value);
    auto map_hash = state.sha1;
    auto immutable_json_items = state.immutableJsonItems;

    auto const* ptr = state.map.find(hkey);
    if (ptr) {
      if (ptr->valueHash == hvalue) {
        return TypedPyObjectRef{this}.release();
      }

      xorHashInPlace(map_hash, ptr->valueHash);
      if (ptr->isImmutableJson) {
        --immutable_json_items;
      }
    }

    xorHashInPlace(map_hash, hvalue);
    bool is_immutable_json = isImmutableJsonItem(key, value);
    if (is_immutable_json) {
      ++immutable_json_items;
    }

    return getOrCreate(
               map_hash,
               [&]() {
                 return state.map.insert(std::make_pair(
                     hkey,
                     DictItem{PyObjectRef{key},
                              PyObjectRef{value},
                              hvalue,
                              is_immutable_json}));
               },
               immutable_json_items)
        .release();
  }

  template <bool Raise>
  PyObject* discard(PyObject* key) noexcept {
    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = state.map.find(h);
    if (!ptr) {
      if (Raise) {
        PyErr_SetObject(PyExc_KeyError, PyObjectRef{key}.release());
        return nullptr;
      }
      return TypedPyObjectRef{this}.release();
    } else {
      auto map_hash = state.sha1;
      auto immutable_json_items = state.immutableJsonItems;
      xorHashInPlace(map_hash, ptr->valueHash);
      if (ptr->isImmutableJson) {
        --immutable_json_items;
      }
      return getOrCreate(
                 map_hash,
                 [&]() { return state.map.erase(h); },
                 immutable_json_items)
          .release();
    }
  }

  Py_ssize_t len() {
    return state.map.size();
  }

  PyObject* iterImpl(ImmutableDictIter::Extractor extractor) {
    auto iter = TypedPyObjectRef<ImmutableDictIter>::create(
        &ImmutableDictIter_typeObject);

    if (iter) {
      new (std::addressof(iter->state)) ImmutableDictIter::State();

      iter->state.iter = state.map.begin();
      iter->state.end = state.map.end();
      iter->state.immutableDict = TypedPyObjectRef{this};
      iter->state.extractor = extractor;
    }

    return iter.release();
  }

  PyObject* iter() {
    return iterImpl(&ImmutableDictIter::keyExtractor);
  }
  PyObject* keys(PyObject* /* unused */) {
    return iterImpl(&ImmutableDictIter::keyExtractor);
  }
  PyObject* values(PyObject* /* unused */) {
    return iterImpl(&ImmutableDictIter::valueExtractor);
  }
  PyObject* items(PyObject* /* unused */) {
    return iterImpl(&ImmutableDictIter::itemExtractor);
  }

  PyObject* repr() {
    PyObjectRef result{PyUnicode_FromString("ImmutableDict({"), false};
    PyObjectRef kv_sep, item_sep;

    if (!result) {
      return nullptr;
    }

    if (Py_ReprEnter(reinterpret_cast<PyObject*>(this)) != 0) {
      return nullptr;
    }
    OnDestroy repr_leave{
        [this]() { Py_ReprLeave(reinterpret_cast<PyObject*>(this)); }};

    bool first = true;
    for (auto const& item : state.map) {
      PyObjectRef key{PyObject_Repr(item.second.key.get()), false};
      if (!key) {
        return nullptr;
      }
      PyObjectRef value{PyObject_Repr(item.second.value.get()), false};
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

      if (!kv_sep) {
        kv_sep = PyObjectRef{PyUnicode_FromString(": "), false};
        if (!kv_sep) {
          return nullptr;
        }
      }
      result = PyObjectRef{PyUnicode_Concat(result.get(), key.get()), false};
      if (!result) {
        return nullptr;
      }
      result = PyObjectRef{PyUnicode_Concat(result.get(), kv_sep.get()), false};
      if (!result) {
        return nullptr;
      }
      result = PyObjectRef{PyUnicode_Concat(result.get(), value.get()), false};
      if (!result) {
        return nullptr;
      }

      first = false;
    }

    PyObjectRef end{PyUnicode_FromString("})"), false};
    if (!end) {
      return nullptr;
    }
    result = PyObjectRef{PyUnicode_Concat(result.get(), end.get()), false};

    return result.release();
  }

  PyObject* isImmutableJsonDict(void* /* unused */) {
    return PyObjectRef(state.isImmutableJson ? Py_True : Py_False).release();
  }

  PyObject* meta(void* /* unused */) {
    if (!state.meta) {
      state.meta = PyObjectRef{PyDict_New(), false};
    }
    return PyObjectRef{state.meta}.release();
  }

  template <typename F>
  static TypedPyObjectRef<ImmutableDict>
  getOrCreate(Sha1Hash hash, F&& map_factory, std::size_t is_immutable_items) {
    auto it = lookUpMap.find(hash);
    if (it != lookUpMap.end()) {
      return TypedPyObjectRef{it->second};
    }

    auto obj =
        TypedPyObjectRef<ImmutableDict>::create(&ImmutableDict_typeObject);

    if (obj) {
      obj->weakreflist = nullptr;
      obj->hasState = false;

      try {
        new (std::addressof(obj->state))
            State(std::forward<F>(map_factory)(), hash, is_immutable_items);
        obj->hasState = true;
        lookUpMap.emplace(hash, obj.get());
      } catch (...) {
        if (obj->hasState) {
          obj->state.~State();
          obj->hasState = false;
        }
        obj = {};
      }
    }

    return obj;
  }

  static PyObject* new_(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Sha1Hash map_hash{0};
    std::size_t immutable_json_items = 0;
    MapType map;

    if (!updateCommon(
            map_hash, immutable_json_items, map, args, kwds, "ImmutableDict")) {
      return nullptr;
    }

    return getOrCreate(
               map_hash, [&]() { return std::move(map); }, immutable_json_items)
        .release();
  }

  PyObject* update(PyObject* args, PyObject* kwds) {
    auto map_hash = state.sha1;
    auto map = state.map;
    auto immutable_json_items = state.immutableJsonItems;

    if (!updateCommon(
            map_hash, immutable_json_items, map, args, kwds, "update")) {
      return nullptr;
    }

    return getOrCreate(
               map_hash, [&]() { return std::move(map); }, immutable_json_items)
        .release();
  }

  static bool updateCommon(
      Sha1Hash& hash,
      std::size_t& immutable_json_items,
      MapType& map,
      PyObject* args,
      PyObject* kwds,
      char const* methname) {
    PyObject* arg = nullptr;

    if (!PyArg_UnpackTuple(args, methname, 0, 1, &arg)) {
      return false;
    }

    if (arg) {
      _Py_IDENTIFIER(keys);
      PyObject* func = nullptr;
      if (_PyObject_LookupAttrId(arg, &PyId_keys, &func) < 0) {
        return false;
      }
      if (func) {
        Py_DECREF(func);
        if (!merge(hash, immutable_json_items, map, arg)) {
          return false;
        }
      } else {
        if (!mergeFromSequence(hash, immutable_json_items, map, arg)) {
          return false;
        }
      }
    }

    if (kwds) {
      if (PyArg_ValidateKeywordArguments(kwds)) {
        if (!merge(hash, immutable_json_items, map, kwds)) {
          return false;
        }
      } else {
        return false;
      }
    }

    return true;
  }

  static bool merge(
      Sha1Hash& hash,
      std::size_t& immutable_json_items,
      MapType& map,
      PyObject* arg) {
    PyObjectRef keys{PyMapping_Keys(arg), false};
    if (!keys) {
      return false;
    }

    PyObjectRef iter{PyObject_GetIter(arg), false};
    if (!iter) {
      return false;
    }
    keys = {};

    while (auto key = PyObjectRef{PyIter_Next(iter.get()), false}) {
      PyObjectRef value{PyObject_GetItem(arg, key.get()), false};
      if (!value) {
        return false;
      }

      map_set(hash, immutable_json_items, map, key.get(), value.get());
    }

    return !PyErr_Occurred();
  }

  static bool mergeFromSequence(
      Sha1Hash& hash,
      std::size_t& immutable_json_items,
      MapType& map,
      PyObject* arg) {
    PyObjectRef iter{PyObject_GetIter(arg), false};
    if (!iter) {
      return false;
    }

    while (auto kv = PyObjectRef{PyIter_Next(iter.get()), false}) {
      PyObjectRef kvseq{PySequence_Fast(kv.get(), ""), false};
      if (!kvseq) {
        return false;
      }

      if (PySequence_Fast_GET_SIZE(kvseq.get()) != 2) {
        PyErr_SetString(
            PyExc_ValueError,
            "all dictionary update sequences must have length of 2");
        return false;
      }

      map_set(
          hash,
          immutable_json_items,
          map,
          PySequence_Fast_GET_ITEM(kvseq.get(), 0),
          PySequence_Fast_GET_ITEM(kvseq.get(), 1));
    }

    return !PyErr_Occurred();
  }

  static void map_set(
      Sha1Hash& hash,
      std::size_t& immutable_json_items,
      MapType& map,
      PyObject* key,
      PyObject* value) {
    auto const [hkey, hvalue] = keyValueHashes(key, value);

    auto const* ptr = map.find(hkey);
    if (ptr) {
      if (ptr->valueHash == hvalue) {
        return;
      }

      xorHashInPlace(hash, ptr->valueHash);
    }

    xorHashInPlace(hash, hvalue);
    bool is_immutable_json = isImmutableJsonItem(key, value);
    if (is_immutable_json) {
      ++immutable_json_items;
    }

    map = map.insert(std::make_pair(
        hkey,
        DictItem{
            PyObjectRef{key}, PyObjectRef{value}, hvalue, is_immutable_json}));
  }

  static void destroy(PyObject* pyself) {
    ImmutableDict* const self = reinterpret_cast<ImmutableDict*>(pyself);

    if (self->weakreflist) {
      PyObject_ClearWeakRefs(pyself);
    }

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

std::unordered_map<Sha1Hash, ImmutableDict*, Sha1HashHasher>
    ImmutableDict::lookUpMap{};

PyMethodDef ImmutableDict_methods[] = {
    {"get",
     method<ImmutableDict, &ImmutableDict::get>(),
     METH_VARARGS,
     "docstring"},
    {"set",
     method<ImmutableDict, &ImmutableDict::set>(),
     METH_VARARGS,
     "docstring"},
    {"discard",
     method<ImmutableDict, &ImmutableDict::discard<false>>(),
     METH_O,
     "docstring"},
    {"pop",
     method<ImmutableDict, &ImmutableDict::discard<true>>(),
     METH_O,
     "docstring"},
    {"update",
     reinterpret_cast<PyCFunction>(static_cast<PyCFunctionWithKeywords>(
         method<ImmutableDict, &ImmutableDict::update>())),
     METH_VARARGS | METH_KEYWORDS,
     "docstring"},
    {"keys",
     method<ImmutableDict, &ImmutableDict::keys>(),
     METH_NOARGS,
     "docstring"},
    {"values",
     method<ImmutableDict, &ImmutableDict::values>(),
     METH_NOARGS,
     "docstring"},
    {"items",
     method<ImmutableDict, &ImmutableDict::items>(),
     METH_NOARGS,
     "docstring"},
    {"_get_instance_count",
     &ImmutableDict::getInstanceCount,
     METH_NOARGS | METH_STATIC,
     "docstring"},
    {nullptr}};

PyMappingMethods ImmutableDict_mappingMethods = {
    .mp_length = method<ImmutableDict, &ImmutableDict::len>(),
    .mp_subscript = method<ImmutableDict, &ImmutableDict::getItem>(),
    .mp_ass_subscript = nullptr,
};

PyGetSetDef ImmutableDict_getset[] = {
    {"isImmutableJson",
     method<ImmutableDict, &ImmutableDict::isImmutableJsonDict>(),
     nullptr,
     "docstring",
     nullptr},
    {"meta",
     method<ImmutableDict, &ImmutableDict::meta>(),
     nullptr,
     "docstring",
     nullptr},
    {nullptr}};

} // namespace

bool isImmutableJsonDict(PyObject* obj) {
  return reinterpret_cast<ImmutableDict*>(obj)->state.isImmutableJson;
}

PyTypeObject ImmutableDict_typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableDict",
    .tp_basicsize = sizeof(ImmutableDict),
    .tp_dealloc = &ImmutableDict::destroy,
    .tp_repr = method<ImmutableDict, &ImmutableDict::repr>(),
    .tp_as_mapping = &ImmutableDict_mappingMethods,
    .tp_doc = "(to be written)",
    .tp_weaklistoffset = offsetof(ImmutableDict, weakreflist),
    .tp_iter = method<ImmutableDict, &ImmutableDict::iter>(),
    .tp_methods = ImmutableDict_methods,
    .tp_getset = ImmutableDict_getset,
    .tp_new = &ImmutableDict::new_,
};
PyTypeObject ImmutableDictIter_typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableDictIterator",
    .tp_basicsize = sizeof(ImmutableDictIter),
    .tp_dealloc = &ImmutableDictIter::destroy,
    .tp_iter = [](PyObject* self) { return PyObjectRef{self}.release(); },
    .tp_iternext = method<ImmutableDictIter, &ImmutableDictIter::next>(),
    .tp_new = &disallow_construction,
};

} // namespace pyimmutable
