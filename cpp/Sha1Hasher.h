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

#pragma once

#include "Sha1Hash.h"

namespace pyimmutable {

class Sha1Hasher : public sha1::Context {
 public:
  Sha1Hasher& operator()(void const* data, std::size_t len) {
    sha1::Context::operator()(data, len);
    return *this;
  }

  Sha1Hasher& operator()(PyObject* obj) {
    if (PyUnicode_Check(obj)) {
      Py_ssize_t len = PyUnicode_GET_LENGTH(obj) * PyUnicode_KIND(obj);
      return (*this)("unc", 3)(static_cast<void const*>(&len), sizeof(len))(
          PyUnicode_DATA(obj), len);
    } else if (PyBytes_Check(obj)) {
      char* data = nullptr;
      Py_ssize_t len = 0;
      PyBytes_AsStringAndSize(obj, &data, &len);
      return (*this)("byt", 3)(data, len);
    } else if (PyLong_Check(obj)) {
      PyLongObject* const lobj = reinterpret_cast<PyLongObject*>(obj);

      Py_ssize_t l = Py_SIZE(lobj);
      (*this)("lon", 3)(&l, sizeof(l));
      if (l > 0) {
        (*this)(lobj->ob_digit, l * sizeof(lobj->ob_digit[0]));
      } else if (l < 0) {
        (*this)(lobj->ob_digit, -l * sizeof(lobj->ob_digit[0]));
      }
      return (*this);
    } else if (PyFloat_Check(obj)) {
      double d = PyFloat_AS_DOUBLE(obj);
      return (*this)("flt", 3)(&d, sizeof(d));
    } else if (PyTuple_Check(obj)) {
      auto const size = PyTuple_Size(obj);
      (*this)("tpl", 3)(&size, sizeof(size));

      for (Py_ssize_t i = 0; i < size; ++i) {
        (*this)(PyTuple_GetItem(obj, i));
      }
      return *this;
    } else {
      return (*this)("obj", 3)(&obj, sizeof(PyObject*));
    }
  }

  static bool objectsEqual(PyObject* p, PyObject* q) {
    if (p == q) {
      // same object, so obviously equal
      return true;
    }

    if (Py_TYPE(p) != Py_TYPE(q)) {
      // different types, objects can't be equal
      return false;
    }

    if (PyUnicode_Check(p) || PyLong_Check(p) || PyFloat_Check(p) ||
        PyTuple_Check(p)) {
      // the objects are of an immutable builtin type for which we check
      // the contents
      return Sha1Hasher()(p).final() == Sha1Hasher()(q).final();
    }

    // ...for all other types we require object identity, and we checked for
    // that already at the top.
    return false;
  }
};

inline std::pair<Sha1Hash, Sha1Hash> keyValueHashes(
    PyObject* key,
    PyObject* value) {
  Sha1Hasher hasher;
  hasher(key);
  Sha1Hasher value_hasher{hasher};
  value_hasher(value);
  return {hasher.final(), value_hasher.final()};
}

inline Sha1Hash valueHash(PyObject* value) {
  return Sha1Hasher{}(value).final();
}

inline Sha1Hash itemHash(Sha1Hash const& value_hash, std::size_t idx) {
  return Sha1Hasher{}("itm", 3)(&idx, sizeof(idx))(
             value_hash.data(), value_hash.size())
      .final();
}

} // namespace pyimmutable
