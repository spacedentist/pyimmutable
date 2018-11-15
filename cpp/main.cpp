#include <Python.h>

#include "ImmutableDict.h"
#include "ImmutableList.h"

static PyMethodDef methods[] = {{nullptr, nullptr, 0, nullptr}};

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
