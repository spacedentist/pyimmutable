#include <Python.h>

namespace pyimmutable {

extern PyTypeObject ImmutableList_typeObject;
extern PyTypeObject ImmutableListIter_typeObject;

bool isImmutableJsonList(PyObject*);

} // namespace pyimmutable
