#include <Python.h>

namespace pyimmutable {

extern PyTypeObject ImmutableDict_typeObject;
extern PyTypeObject ImmutableDictKeyIter_typeObject;
extern PyTypeObject ImmutableDictValueIter_typeObject;
extern PyTypeObject ImmutableDictItemIter_typeObject;

bool isImmutableJsonDict(PyObject*);

} // namespace pyimmutable
