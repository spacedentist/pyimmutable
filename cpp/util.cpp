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

#include "util.h"
#include "ImmutableDict.h"
#include "ImmutableList.h"

namespace pyimmutable {

bool isImmutableJsonObject(PyObject* obj) {
  if ((obj == Py_None) || PyUnicode_CheckExact(obj) || PyBool_Check(obj) ||
      PyLong_CheckExact(obj) || PyFloat_CheckExact(obj)) {
    return true;
  }

  if (Py_TYPE(obj) == immutableDictTypeObject) {
    return isImmutableJsonDict(obj);
  }
  if (Py_TYPE(obj) == immutableListTypeObject) {
    return isImmutableJsonList(obj);
  }

  return false;
}

PyObject* disallow_construction(
    PyTypeObject* /*type*/,
    PyObject* /*args*/,
    PyObject*
    /*kwds*/) {
  PyErr_SetString(PyExc_TypeError, "cannot create instances of this type");
  return nullptr;
}

} // namespace pyimmutable
