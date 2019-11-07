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

#include "ImmutableList.h"

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

#include "ClassWrapper.h"
#include "PyObjectRef.h"
#include "Sha1Hasher.h"
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
  using Wrapper = ClassWrapper<ImmutableListIter>;

  VectorType::const_iterator iter;
  VectorType::const_iterator end;
  PyObjectRef immutableList;
  bool reversed{false};

  ImmutableListIter(
      VectorType::const_iterator iter,
      VectorType::const_iterator end,
      PyObjectRef immutableList,
      bool reversed)
      : iter(iter),
        end(end),
        immutableList(std::move(immutableList)),
        reversed(reversed) {}

  PyObjectRef next() {
    if (iter == end) {
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
    return (reversed ? (--end) : (iter++))->value.copy();
  }
};

struct ImmutableList {
  using Wrapper = ClassWrapper<ImmutableList>;
  static constexpr bool weakrefs_enabled = true;
  static constexpr bool sha1_lookup_enabled = true;

  VectorType vec;
  Sha1Hash const sha1;
  std::size_t const immutableJsonItems;
  bool const isImmutableJson;
  PyObjectRef meta_;

  ImmutableList(
      VectorType&& vecx,
      Sha1Hash sha1,
      std::size_t immutable_json_items)
      : vec(std::move(vecx)),
        sha1(sha1),
        immutableJsonItems(immutable_json_items),
        isImmutableJson(immutableJsonItems == vec.size()) {}

  PyObjectRef getItemIdx(Py_ssize_t idx) noexcept {
    Py_ssize_t const len = vec.size();

    if (len == 0 || idx >= len || idx < -len) {
      PyErr_SetNone(PyExc_IndexError);
      return nullptr;
    } else {
      if (idx < 0) {
        idx = len + idx;
      }
      return vec[idx].value.copy();
    }
  }

  PyObjectRef getItem(PyObject* item) noexcept {
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
      return makeSlice(start, stop, step);
    }

    PyErr_Format(
        PyExc_TypeError,
        "range indices must be integers or slices, not %.200s",
        item->ob_type->tp_name);
    return nullptr;
  }

  PyObjectRef makeSlice(Py_ssize_t start, Py_ssize_t stop, Py_ssize_t step) {
    if (step == 0) {
      return {};
    }

    Py_ssize_t length = PySlice_AdjustIndices(vec.size(), &start, &stop, step);

    if (length <= 0) {
      return makeEmpty();
    }

    if (start == 0 && step == 1) {
      if (length >= static_cast<Py_ssize_t>(vec.size())) {
        return TypedPyObjectRef{Wrapper::cast(this)};
      }

      Sha1Hash hash{0};
      std::size_t immutable_json_items = 0;
      for (auto it = vec.begin(); start < stop; ++start, ++it) {
        xorHashInPlace(hash, it->itemHash);
        if (it->isImmutableJson) {
          ++immutable_json_items;
        }
      }

      return Wrapper::getOrCreate(hash, [&]() {
        return ImmutableList{vec.take(length), hash, immutable_json_items};
      });
    }

    Sha1Hash hash{0};
    std::size_t immutable_json_items = 0;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(length);

    for (Py_ssize_t i = 0; i < length; ++i) {
      auto const& src_item = vec[start + i * step];
      auto item_hash = itemHash(src_item.valueHash, i);
      item_hashes.push_back(item_hash);
      xorHashInPlace(hash, item_hash);
      if (src_item.isImmutableJson) {
        ++immutable_json_items;
      }
    }

    return Wrapper::getOrCreate(hash, [&]() {
      TransientVectorType tvec;
      for (Py_ssize_t i = 0; i < length; ++i) {
        auto const& src_item = vec[start + i * step];
        auto const& item_hash = item_hashes[i];
        tvec.push_back(ListItem{src_item.value,
                                src_item.valueHash,
                                item_hash,
                                src_item.isImmutableJson});
      }

      return ImmutableList{
          std::move(tvec).persistent(), hash, immutable_json_items};
    });
  }

  PyObjectRef set(Py_ssize_t idx, PyObject* value) noexcept {
    if (idx < 0) {
      idx += vec.size();
    }
    if (idx < 0 or idx >= static_cast<Py_ssize_t>(vec.size())) {
      PyErr_SetNone(PyExc_IndexError);
      return nullptr;
    }

    auto const& src_item = vec[idx];
    auto const hvalue = valueHash(value);

    if (hvalue == src_item.valueHash) {
      return TypedPyObjectRef{Wrapper::cast(this)};
    }

    auto const hitem = itemHash(hvalue, idx);
    auto const vec_hash = xorHash(xorHash(sha1, src_item.itemHash), hitem);
    bool const is_immutable_json = isImmutableJsonObject(value);
    auto const immutable_json_items = immutableJsonItems -
        (src_item.isImmutableJson ? 1 : 0) + (is_immutable_json ? 1 : 0);

    return Wrapper::getOrCreate(vec_hash, [&]() {
      return ImmutableList{
          vec.set(
              idx,
              ListItem{
                  PyObjectRef{value}, hvalue, hitem, immutable_json_items}),
          vec_hash,
          immutable_json_items};
    });
  }

  auto append(PyObject* value) noexcept {
    auto const hvalue = valueHash(value);
    auto const hitem = itemHash(hvalue, vec.size());
    auto vec_hash = xorHash(sha1, hitem);
    bool const is_immutable_json = isImmutableJsonObject(value);
    auto const immutable_json_items =
        immutableJsonItems + (is_immutable_json ? 1 : 0);

    return Wrapper::getOrCreate(vec_hash, [&]() {
      return ImmutableList{
          vec.push_back(
              ListItem{PyObjectRef{value}, hvalue, hitem, is_immutable_json}),
          vec_hash,
          immutable_json_items};
    });
  }

  PyObjectRef count(PyObject* value) noexcept {
    std::size_t count = 0;

    for (auto const& item : vec) {
      if (item.value.get() == value ||
          Sha1Hasher::objectsEqual(item.value.get(), value)) {
        ++count;
      }
    }

    return PyObjectRef{PyLong_FromSize_t(count), false};
  }

  int contains(PyObject* value) noexcept {
    for (auto const& item : vec) {
      if (item.value.get() == value ||
          Sha1Hasher::objectsEqual(item.value.get(), value)) {
        return 1;
      }
    }

    return 0;
  }

  PyObjectRef
  index(PyObject* value, Py_ssize_t start, Py_ssize_t stop) noexcept {
    if (start < 0) {
      start = std::max<Py_ssize_t>(vec.size() + start, 0);
    }
    if (stop < 0) {
      stop += vec.size();
    }
    stop = std::min<Py_ssize_t>(stop, vec.size());

    for (auto i = start; i < stop; ++i) {
      auto const& item = vec[i];
      if (item.value.get() == value ||
          Sha1Hasher::objectsEqual(item.value.get(), value)) {
        return PyObjectRef{PyLong_FromSize_t(i), false};
      }
    }

    PyErr_Format(PyExc_ValueError, "%R is not in ImmutableList", value);
    return nullptr;
  }

  Py_ssize_t len() {
    return vec.size();
  }

  PyObjectRef concat(PyObject* rhs_ptr) {
    if (Py_TYPE(rhs_ptr) != immutableListTypeObject) {
      PyErr_Format(
          PyExc_TypeError,
          "can only concatenate ImmutableList "
          "(not \"%.200s\") to ImmutableList",
          rhs_ptr->ob_type->tp_name);
      return nullptr;
    }

    ImmutableList& rhs = *Wrapper::cast(rhs_ptr);

    if (rhs.vec.empty()) {
      return PyObjectRef{Wrapper::pyObject(this)};
    }

    if (vec.empty()) {
      return PyObjectRef{rhs_ptr};
    }

    Py_ssize_t idx = vec.size();
    Sha1Hash hash = sha1;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(rhs.vec.size());

    for (auto const& item : rhs.vec) {
      auto hitem = itemHash(item.valueHash, idx++);
      item_hashes.push_back(hitem);
      xorHashInPlace(hash, hitem);
    }

    return Wrapper::getOrCreate(hash, [&]() {
      auto tvec = vec.transient();

      auto it = item_hashes.begin();
      for (auto const& item : rhs.vec) {
        tvec.push_back(
            ListItem{item.value, item.valueHash, *it++, item.isImmutableJson});
      }

      return ImmutableList{std::move(tvec).persistent(),
                           hash,
                           immutableJsonItems + rhs.immutableJsonItems};
    });
  }

  PyObjectRef repeat(Py_ssize_t count) {
    if (count <= 0 or vec.empty()) {
      return makeEmpty();
    }
    if (count == 1) {
      return TypedPyObjectRef{Wrapper::cast(this)};
    }

    Py_ssize_t idx = vec.size();
    Sha1Hash hash = sha1;
    std::vector<Sha1Hash> item_hashes;
    item_hashes.reserve(idx * (count - 1));

    for (Py_ssize_t c = 1; c < count; ++c) {
      for (auto const& item : vec) {
        auto hitem = itemHash(item.valueHash, idx++);
        item_hashes.push_back(hitem);
        xorHashInPlace(hash, hitem);
      }
    }

    return Wrapper::getOrCreate(hash, [&]() {
      auto tvec = vec.transient();

      auto it = item_hashes.begin();
      for (Py_ssize_t c = 1; c < count; ++c) {
        for (auto const& item : vec) {
          tvec.push_back(ListItem{
              item.value, item.valueHash, *it++, item.isImmutableJson});
        }
      }

      return ImmutableList{
          std::move(tvec).persistent(), hash, immutableJsonItems * count};
    });
  }

  auto iterImpl(bool reversed) {
    return ImmutableListIter::Wrapper::create(
        vec.begin(),
        vec.end(),
        TypedPyObjectRef{Wrapper::cast(this)},
        reversed);
  }

  PyObjectRef iter() {
    return iterImpl(false);
  }

  PyObjectRef repr() {
    PyObjectRef result{PyUnicode_FromString("ImmutableList(["), false};
    PyObjectRef item_sep;

    if (!result) {
      return nullptr;
    }

    if (Py_ReprEnter(Wrapper::pyObject(this)) != 0) {
      return nullptr;
    }
    OnDestroy repr_leave{[this]() { Py_ReprLeave(Wrapper::pyObject(this)); }};

    bool first = true;
    for (auto const& item : vec) {
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

    return PyObjectRef{PyUnicode_Concat(result.get(), end.get()), false};
  }

  PyObjectRef isImmutableJsonList(void* /* unused */) {
    return PyObjectRef(isImmutableJson ? Py_True : Py_False);
  }

  PyObjectRef meta(void* /* unused */) {
    if (!meta_) {
      meta_ = PyObjectRef{PyDict_New(), false};
    }
    return meta_;
  }

  static PyObjectRef makeEmpty() {
    return Wrapper::getOrCreate({}, []() { return ImmutableList({}, {}, 0); });
  }

  static PyObjectRef new_(PyTypeObject* type, PyObject* args, PyObject* kwds) {
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
      return Wrapper::getOrCreate(vec_hash, [&]() {
        return ImmutableList{
            std::move(tvec).persistent(), vec_hash, immutable_json_items};
      });
    } else {
      return makeEmpty();
    }
  }

  PyObjectRef extend(PyObject* seq) noexcept {
    if (Py_TYPE(seq) == immutableListTypeObject) {
      // optimisation: if argument is ImmutableList we can use concat,
      // which does not have to compute value hashes
      return concat(seq);
    }

    auto vec_hash = sha1;
    auto immutable_json_items = immutableJsonItems;
    auto tvec = vec.transient();
    if (!extendCommon(vec_hash, immutable_json_items, tvec, seq)) {
      return nullptr;
    }
    return Wrapper::getOrCreate(vec_hash, [&]() {
      return ImmutableList{
          std::move(tvec).persistent(), vec_hash, immutable_json_items};
    });
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
};

} // namespace

} // namespace pyimmutable
