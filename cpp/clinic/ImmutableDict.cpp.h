/*[clinic input]
preserve
[clinic start generated code]*/

PyDoc_STRVAR(_pyimmutable_ImmutableDict__get_instance_count__doc__,
"_get_instance_count()\n"
"--\n"
"\n"
"Return the number of ``ImmutableDict`` objects currently in existence.\n"
"\n"
"This is mainly useful for the ``ImmutableDict`` test suite.");

#define _PYIMMUTABLE_IMMUTABLEDICT__GET_INSTANCE_COUNT_METHODDEF    \
    {"_get_instance_count", (PyCFunction)_pyimmutable_ImmutableDict__get_instance_count, METH_NOARGS|METH_STATIC, _pyimmutable_ImmutableDict__get_instance_count__doc__},

static PyObject *
_pyimmutable_ImmutableDict__get_instance_count_impl();

static PyObject *
_pyimmutable_ImmutableDict__get_instance_count(void *null, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableDict__get_instance_count_impl();
}

PyDoc_STRVAR(_pyimmutable_ImmutableDict_discard__doc__,
"discard($self, key, /)\n"
"--\n"
"\n"
"Return a copy with ``key`` removed.\n"
"\n"
"Returns ``self`` if ``key`` is not present.");

#define _PYIMMUTABLE_IMMUTABLEDICT_DISCARD_METHODDEF    \
    {"discard", (PyCFunction)_pyimmutable_ImmutableDict_discard, METH_O, _pyimmutable_ImmutableDict_discard__doc__},

PyDoc_STRVAR(_pyimmutable_ImmutableDict_get__doc__,
"get($self, key, default=None, /)\n"
"--\n"
"\n"
"Return the value for ``key`` if ``key`` is in the dictionary, else ``default``.");

#define _PYIMMUTABLE_IMMUTABLEDICT_GET_METHODDEF    \
    {"get", (PyCFunction)_pyimmutable_ImmutableDict_get, METH_FASTCALL, _pyimmutable_ImmutableDict_get__doc__},

static PyObject *
_pyimmutable_ImmutableDict_get_impl(pyimmutable::ImmutableDict::Wrapper*self,
                                    PyObject *key, PyObject *default_value);

static PyObject *
_pyimmutable_ImmutableDict_get(pyimmutable::ImmutableDict::Wrapper*self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *key;
    PyObject *default_value = Py_None;

    if (!_PyArg_UnpackStack(args, nargs, "get",
        1, 2,
        &key, &default_value)) {
        goto exit;
    }
    return_value = _pyimmutable_ImmutableDict_get_impl(self, key, default_value);

exit:
    return return_value;
}

PyDoc_STRVAR(_pyimmutable_ImmutableDict_items__doc__,
"items($self, /)\n"
"--\n"
"\n"
"Return an iterator over ``(key, value)`` tuples.");

#define _PYIMMUTABLE_IMMUTABLEDICT_ITEMS_METHODDEF    \
    {"items", (PyCFunction)_pyimmutable_ImmutableDict_items, METH_NOARGS, _pyimmutable_ImmutableDict_items__doc__},

static PyObject *
_pyimmutable_ImmutableDict_items_impl(pyimmutable::ImmutableDict::Wrapper*self);

static PyObject *
_pyimmutable_ImmutableDict_items(pyimmutable::ImmutableDict::Wrapper*self, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableDict_items_impl(self);
}

PyDoc_STRVAR(_pyimmutable_ImmutableDict_keys__doc__,
"keys($self, /)\n"
"--\n"
"\n"
"Return an iterator over the keys in this ``ImmutableDict``.");

#define _PYIMMUTABLE_IMMUTABLEDICT_KEYS_METHODDEF    \
    {"keys", (PyCFunction)_pyimmutable_ImmutableDict_keys, METH_NOARGS, _pyimmutable_ImmutableDict_keys__doc__},

static PyObject *
_pyimmutable_ImmutableDict_keys_impl(pyimmutable::ImmutableDict::Wrapper*self);

static PyObject *
_pyimmutable_ImmutableDict_keys(pyimmutable::ImmutableDict::Wrapper*self, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableDict_keys_impl(self);
}

PyDoc_STRVAR(_pyimmutable_ImmutableDict_pop__doc__,
"pop($self, key, /)\n"
"--\n"
"\n"
"Return a copy with ``key`` removed.\n"
"\n"
"Raises ``KeyError`` if ``key`` is not present.");

#define _PYIMMUTABLE_IMMUTABLEDICT_POP_METHODDEF    \
    {"pop", (PyCFunction)_pyimmutable_ImmutableDict_pop, METH_O, _pyimmutable_ImmutableDict_pop__doc__},

PyDoc_STRVAR(_pyimmutable_ImmutableDict_set__doc__,
"set($self, key, value, /)\n"
"--\n"
"\n"
"Return a copy with ``key`` set to ``value``.");

#define _PYIMMUTABLE_IMMUTABLEDICT_SET_METHODDEF    \
    {"set", (PyCFunction)_pyimmutable_ImmutableDict_set, METH_FASTCALL, _pyimmutable_ImmutableDict_set__doc__},

static PyObject *
_pyimmutable_ImmutableDict_set_impl(pyimmutable::ImmutableDict::Wrapper*self,
                                    PyObject *key, PyObject *value);

static PyObject *
_pyimmutable_ImmutableDict_set(pyimmutable::ImmutableDict::Wrapper*self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *key;
    PyObject *value;

    if (!_PyArg_UnpackStack(args, nargs, "set",
        2, 2,
        &key, &value)) {
        goto exit;
    }
    return_value = _pyimmutable_ImmutableDict_set_impl(self, key, value);

exit:
    return return_value;
}

PyDoc_STRVAR(_pyimmutable_ImmutableDict_values__doc__,
"values($self, /)\n"
"--\n"
"\n"
"Return an iterator over the values in this ``ImmutableDict``.");

#define _PYIMMUTABLE_IMMUTABLEDICT_VALUES_METHODDEF    \
    {"values", (PyCFunction)_pyimmutable_ImmutableDict_values, METH_NOARGS, _pyimmutable_ImmutableDict_values__doc__},

static PyObject *
_pyimmutable_ImmutableDict_values_impl(pyimmutable::ImmutableDict::Wrapper*self);

static PyObject *
_pyimmutable_ImmutableDict_values(pyimmutable::ImmutableDict::Wrapper*self, PyObject *Py_UNUSED(ignored))
{
    return _pyimmutable_ImmutableDict_values_impl(self);
}
/*[clinic end generated code: output=1758767da8748747 input=a9049054013a1b77]*/
