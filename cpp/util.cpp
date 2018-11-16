#include "util.h"
#include "ImmutableDict.h"
#include "ImmutableList.h"

namespace pyimmutable {

bool isImmutableJsonObject(PyObject* obj) {
  return (obj == Py_None) || PyUnicode_CheckExact(obj) || PyBool_Check(obj) ||
      PyLong_CheckExact(obj) || PyFloat_CheckExact(obj) ||
      (Py_TYPE(obj) == &ImmutableDict_typeObject && isImmutableJsonDict(obj)) ||
      (Py_TYPE(obj) == &ImmutableList_typeObject && isImmutableJsonList(obj));
}

} // namespace pyimmutable
