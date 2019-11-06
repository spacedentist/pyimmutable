/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(_pyimmutable_ImmutableList___reversed____doc__,
"__reversed__($self, /)\n"
"--\n"
"\n"
"Return a reverse iterator over the ``ImmutableList``.");

#define _PYIMMUTABLE_IMMUTABLELIST___REVERSED___METHODDEF    \
    {"__reversed__", (PyCFunction)_pyimmutable_ImmutableList___reversed__, METH_NOARGS, _pyimmutable_ImmutableList___reversed____doc__},

static PyObject *
_pyimmutable_ImmutableList___reversed___impl(pyimmutable::ImmutableList::Wrapper*self);

static PyObject *
_pyimmutable_ImmutableList___reversed__(pyimmutable::ImmutableList::Wrapper*self, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableList___reversed___impl(self);
}

PyDoc_STRVAR(_pyimmutable_ImmutableList__get_instance_count__doc__,
"_get_instance_count()\n"
"--\n"
"\n"
"Return the number of ``ImmutableList`` objects currently in existence.\n"
"\n"
"This is mainly useful for the ``ImmutableList`` test suite.");

#define _PYIMMUTABLE_IMMUTABLELIST__GET_INSTANCE_COUNT_METHODDEF    \
    {"_get_instance_count", (PyCFunction)_pyimmutable_ImmutableList__get_instance_count, METH_NOARGS|METH_STATIC, _pyimmutable_ImmutableList__get_instance_count__doc__},

static PyObject *
_pyimmutable_ImmutableList__get_instance_count_impl();

static PyObject *
_pyimmutable_ImmutableList__get_instance_count(void *null, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableList__get_instance_count_impl();
}

PyDoc_STRVAR(_pyimmutable_ImmutableList_append__doc__,
"append($self, value, /)\n"
"--\n"
"\n"
"Return a copy with the given ``value`` appended.");

#define _PYIMMUTABLE_IMMUTABLELIST_APPEND_METHODDEF    \
    {"append", (PyCFunction)_pyimmutable_ImmutableList_append, METH_O, _pyimmutable_ImmutableList_append__doc__},

PyDoc_STRVAR(_pyimmutable_ImmutableList_count__doc__,
"count($self, value, /)\n"
"--\n"
"\n"
"Return number of occurrences of ``value``.");

#define _PYIMMUTABLE_IMMUTABLELIST_COUNT_METHODDEF    \
    {"count", (PyCFunction)_pyimmutable_ImmutableList_count, METH_O, _pyimmutable_ImmutableList_count__doc__},

PyDoc_STRVAR(_pyimmutable_ImmutableList_extend__doc__,
"extend($self, iterable, /)\n"
"--\n"
"\n"
"Return a copy extended by appending elements from ``iterable``.");

#define _PYIMMUTABLE_IMMUTABLELIST_EXTEND_METHODDEF    \
    {"extend", (PyCFunction)_pyimmutable_ImmutableList_extend, METH_O, _pyimmutable_ImmutableList_extend__doc__},

PyDoc_STRVAR(_pyimmutable_ImmutableList_index__doc__,
"index($self, value, start=0, stop=sys.maxsize, /)\n"
"--\n"
"\n"
"Return first index of ``value``.\n"
"\n"
"Raises ``ValueError`` if ``value`` is not present.");

#define _PYIMMUTABLE_IMMUTABLELIST_INDEX_METHODDEF    \
    {"index", (PyCFunction)_pyimmutable_ImmutableList_index, METH_FASTCALL, _pyimmutable_ImmutableList_index__doc__},

static PyObject *
_pyimmutable_ImmutableList_index_impl(pyimmutable::ImmutableList::Wrapper*self,
                                      PyObject *value, Py_ssize_t start,
                                      Py_ssize_t stop);

static PyObject *
_pyimmutable_ImmutableList_index(pyimmutable::ImmutableList::Wrapper*self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *value;
    Py_ssize_t start = 0;
    Py_ssize_t stop = PY_SSIZE_T_MAX;

    if (!_PyArg_ParseStack(args, nargs, "O|O&O&:index",
        &value, _PyEval_SliceIndexNotNone, &start, _PyEval_SliceIndexNotNone, &stop)) {
        goto exit;
    }
    return_value = _pyimmutable_ImmutableList_index_impl(self, value, start, stop);

exit:
    return return_value;
}

PyDoc_STRVAR(_pyimmutable_ImmutableList_set__doc__,
"set($self, index, value, /)\n"
"--\n"
"\n"
"Return a copy with item ``index`` set to ``value``.\n"
"\n"
"Raises ``IndexError`` if ``index`` is outside the range of existing elements.");

#define _PYIMMUTABLE_IMMUTABLELIST_SET_METHODDEF    \
    {"set", (PyCFunction)_pyimmutable_ImmutableList_set, METH_FASTCALL, _pyimmutable_ImmutableList_set__doc__},

static PyObject *
_pyimmutable_ImmutableList_set_impl(pyimmutable::ImmutableList::Wrapper*self,
                                    Py_ssize_t index, PyObject *value);

static PyObject *
_pyimmutable_ImmutableList_set(pyimmutable::ImmutableList::Wrapper*self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    Py_ssize_t index;
    PyObject *value;

    if (!_PyArg_ParseStack(args, nargs, "nO:set",
        &index, &value)) {
        goto exit;
    }
    return_value = _pyimmutable_ImmutableList_set_impl(self, index, value);

exit:
    return return_value;
}
/*[clinic end generated code: output=318b40802275b56b input=a9049054013a1b77]*/
