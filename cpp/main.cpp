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

#include <Python.h>

#include "ClassWrapper.h"
#include "ImmutableDict.h"
#include "ImmutableList.h"
#include "PyObjectRef.h"
#include "util.h"

static PyMethodDef methods[] = {
    {"isImmutableJson",
     [](PyObject*, PyObject* obj) {
       bool value = pyimmutable::isImmutableJsonObject(obj);
       return pyimmutable::PyObjectRef{value ? Py_True : Py_False}.release();
     },
     METH_O,
     nullptr},
    {nullptr, nullptr, 0, nullptr}};

static struct PyModuleDef module = {PyModuleDef_HEAD_INIT,
                                    "pyimmutable",
                                    "docstring",
                                    -1,
                                    methods};

PyTypeObject* pyimmutable::immutableDictTypeObject{nullptr};
PyTypeObject* pyimmutable::immutableListTypeObject{nullptr};

extern "C" {
PyMODINIT_FUNC PyInit__pyimmutable(void) {
  using namespace pyimmutable;

  immutableDictTypeObject = getImmutableDictTypeObject();
  if (!immutableDictTypeObject) {
    return nullptr;
  }

  immutableListTypeObject = getImmutableListTypeObject();
  if (!immutableListTypeObject) {
    return nullptr;
  }

  auto* immutable_dict_iter_type = getImmutableDictIterTypeObject();
  if (!immutable_dict_iter_type) {
    return nullptr;
  }

  auto* immutable_list_iter_type = getImmutableListIterTypeObject();
  if (!immutable_list_iter_type) {
    return nullptr;
  }

  PyObject* m = PyModule_Create(&module);
  if (!m) {
    return nullptr;
  }

  PyModule_AddObject(
      m,
      "ImmutableDict",
      PyObjectRef{reinterpret_cast<PyObject*>(immutableDictTypeObject)}
          .release());

  PyModule_AddObject(
      m,
      "ImmutableList",
      PyObjectRef{reinterpret_cast<PyObject*>(immutableListTypeObject)}
          .release());

  return m;
}
}
