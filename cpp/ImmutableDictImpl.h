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

#include "ClassWrapper.h"
#include "PyObjectRef.h"
#include "Sha1Hasher.h"
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
  using Wrapper = ClassWrapper<ImmutableDictIter>;
  using Extractor = PyObjectRef (*)(DictItem const&);

  MapType::const_iterator iter;
  MapType::const_iterator end;
  PyObjectRef immutableDict;
  Extractor extractor;

  ImmutableDictIter(
      MapType::const_iterator iter,
      MapType::const_iterator end,
      PyObjectRef immutableDict,
      Extractor extractor)
      : iter(iter),
        end(end),
        immutableDict(std::move(immutableDict)),
        extractor(extractor) {}

  PyObjectRef next() {
    if (iter == end) {
      PyErr_SetNone(PyExc_StopIteration);
      return nullptr;
    }
    return extractor(iter++->second).copy();
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
};

struct ImmutableDict {
  using Wrapper = ClassWrapper<ImmutableDict>;
  static constexpr bool weakrefs_enabled = true;
  static constexpr bool sha1_lookup_enabled = true;

  MapType map_;
  Sha1Hash const sha1;
  std::size_t const immutableJsonItems;
  bool const isImmutableJson;
  PyObjectRef meta_;

  ImmutableDict(MapType&& mapx, Sha1Hash sha1, std::size_t immutable_json_items)
      : map_(std::move(mapx)),
        sha1(sha1),
        immutableJsonItems(immutable_json_items),
        isImmutableJson(immutableJsonItems == map_.size()) {}

  PyObjectRef getItem(PyObject* key) noexcept {
    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = map_.find(h);
    if (!ptr) {
      PyErr_SetObject(PyExc_KeyError, PyObjectRef{key}.release());
      return nullptr;
    } else {
      return ptr->value.copy();
    }
  }

  PyObjectRef get(PyObject* key, PyObject* default_value) noexcept {
    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = map_.find(h);
    if (!ptr) {
      return PyObjectRef{default_value};
    } else {
      return ptr->value;
    }
  }

  PyObjectRef set(PyObject* key, PyObject* value) noexcept {
    auto const [hkey, hvalue] = keyValueHashes(key, value);
    auto map_hash = sha1;
    auto immutable_json_items = immutableJsonItems;

    auto const* ptr = map_.find(hkey);
    if (ptr) {
      if (ptr->valueHash == hvalue) {
        return TypedPyObjectRef{Wrapper::cast(this)};
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

    return Wrapper::getOrCreate(map_hash, [&]() {
      return ImmutableDict{map_.insert(std::make_pair(
                               hkey,
                               DictItem{PyObjectRef{key},
                                        PyObjectRef{value},
                                        hvalue,
                                        is_immutable_json})),
                           map_hash,
                           immutable_json_items};
    });
  }

  template <bool Raise>
  PyObjectRef discard(PyObject* key) noexcept {
    auto const h = Sha1Hasher()(key).final();
    auto const* ptr = map_.find(h);
    if (!ptr) {
      if (Raise) {
        PyErr_SetObject(PyExc_KeyError, PyObjectRef{key}.release());
        return nullptr;
      }
      return TypedPyObjectRef{Wrapper::cast(this)};
    } else {
      auto map_hash = sha1;
      auto immutable_json_items = immutableJsonItems;
      xorHashInPlace(map_hash, ptr->valueHash);
      if (ptr->isImmutableJson) {
        --immutable_json_items;
      }
      return Wrapper::getOrCreate(map_hash, [&]() {
        return ImmutableDict{map_.erase(h), map_hash, immutable_json_items};
      });
    }
  }

  Py_ssize_t len() {
    return map_.size();
  }

  PyObjectRef iterImpl(ImmutableDictIter::Extractor extractor) {
    return ImmutableDictIter::Wrapper::create(
        map_.begin(),
        map_.end(),
        TypedPyObjectRef{Wrapper::cast(this)},
        extractor);
  }

  PyObjectRef keys() {
    return iterImpl(&ImmutableDictIter::keyExtractor);
  }
  PyObjectRef values() {
    return iterImpl(&ImmutableDictIter::valueExtractor);
  }
  PyObjectRef items() {
    return iterImpl(&ImmutableDictIter::itemExtractor);
  }

  PyObjectRef repr() {
    PyObjectRef result{PyUnicode_FromString("ImmutableDict({"), false};
    PyObjectRef kv_sep, item_sep;

    if (!result) {
      return nullptr;
    }

    if (Py_ReprEnter(Wrapper::pyObject(this)) != 0) {
      return nullptr;
    }
    OnDestroy repr_leave{[this]() { Py_ReprLeave(Wrapper::pyObject(this)); }};

    bool first = true;
    for (auto const& item : map_) {
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

    return PyObjectRef{PyUnicode_Concat(result.get(), end.get()), false};
  }

  PyObjectRef isImmutableJsonDict(void* /* unused */) {
    return PyObjectRef(isImmutableJson ? Py_True : Py_False);
  }

  PyObjectRef meta(void* /* unused */) {
    if (!meta_) {
      meta_ = PyObjectRef{PyDict_New(), false};
    }
    return meta_;
  }

  static PyObjectRef new_(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Sha1Hash map_hash{0};
    std::size_t immutable_json_items = 0;
    MapType map;

    if (!updateCommon(
            map_hash, immutable_json_items, map, args, kwds, "ImmutableDict")) {
      return nullptr;
    }

    return Wrapper::getOrCreate(map_hash, [&]() {
      return ImmutableDict{std::move(map), map_hash, immutable_json_items};
    });
  }

  PyObjectRef update(PyObject* args, PyObject* kwds) {
    auto map_hash = sha1;
    auto map = map_;
    auto immutable_json_items = immutableJsonItems;

    if (!updateCommon(
            map_hash, immutable_json_items, map, args, kwds, "update")) {
      return nullptr;
    }

    return Wrapper::getOrCreate(map_hash, [&]() {
      return ImmutableDict{std::move(map), map_hash, immutable_json_items};
    });
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

    PyObjectRef iter{PyObject_GetIter(keys.get()), false};
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
};

} // namespace
} // namespace pyimmutable
