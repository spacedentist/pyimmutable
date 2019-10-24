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

extern "C" {
PyMODINIT_FUNC PyInit__pyimmutable(void) {
  if (PyType_Ready(&pyimmutable::ImmutableDict_typeObject) < 0 ||
      PyType_Ready(&pyimmutable::ImmutableDictKeyIter_typeObject) < 0 ||
      PyType_Ready(&pyimmutable::ImmutableDictValueIter_typeObject) < 0 ||
      PyType_Ready(&pyimmutable::ImmutableDictItemIter_typeObject) < 0 ||
      PyType_Ready(&pyimmutable::ImmutableList_typeObject) < 0 ||
      PyType_Ready(&pyimmutable::ImmutableListIter_typeObject) < 0) {
    return nullptr;
  }

  PyObject* m = PyModule_Create(&module);
  if (!m) {
    return nullptr;
  }

  Py_INCREF(&pyimmutable::ImmutableDict_typeObject);
  Py_INCREF(&pyimmutable::ImmutableDictKeyIter_typeObject);
  Py_INCREF(&pyimmutable::ImmutableDictValueIter_typeObject);
  Py_INCREF(&pyimmutable::ImmutableDictItemIter_typeObject);
  PyModule_AddObject(
      m,
      "ImmutableDict",
      reinterpret_cast<PyObject*>(&pyimmutable::ImmutableDict_typeObject));

  Py_INCREF(&pyimmutable::ImmutableList_typeObject);
  Py_INCREF(&pyimmutable::ImmutableListIter_typeObject);
  PyModule_AddObject(
      m,
      "ImmutableList",
      reinterpret_cast<PyObject*>(&pyimmutable::ImmutableList_typeObject));

  return m;
}
}
