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

#include "ImmutableDictImpl.h"
#include "clinic/ImmutableDict.cpp.h"
#include "docstrings.autogen.h"

// clang-format off
/*[clinic input]
module _pyimmutable
class _pyimmutable.ImmutableDict "pyimmutable::ImmutableDict::Wrapper*" "pyimmutable::immutableDictTypeObject"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=8f6d2175d96a4fe2]*/
// clang-format on

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
@staticmethod
_pyimmutable.ImmutableDict._get_instance_count

Return the number of ``ImmutableDict`` objects currently in existence.

This is mainly useful for the ``ImmutableDict`` test suite.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict__get_instance_count_impl()
/*[clinic end generated code: output=45dc5aa5edf23f57 input=81f4677b9275070d]*/
// clang-format on
{
  return PyLong_FromSize_t(
      pyimmutable::ImmutableDict::Wrapper::getInstanceCount());
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.discard

  key: object
  /

Return a copy with ``key`` removed.

Returns ``self`` if ``key`` is not present.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_discard(pyimmutable::ImmutableDict::Wrapper*self,
                                   PyObject *key)
/*[clinic end generated code: output=2a4db12ba8233045 input=063b0b025a515674]*/
// clang-format on
{
  return self->discard<false>(key).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.get

  key: object
  default: object = None
  /

Return the value for ``key`` if ``key`` is in the dictionary, else ``default``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_get_impl(pyimmutable::ImmutableDict::Wrapper*self,
                                    PyObject *key, PyObject *default_value)
/*[clinic end generated code: output=8a0c1ef395aef988 input=b05d2bf759106bff]*/
// clang-format on
{
  return self->get(key, default_value).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.items

Return an iterator over ``(key, value)`` tuples.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_items_impl(pyimmutable::ImmutableDict::Wrapper*self)
/*[clinic end generated code: output=6f9b02a2729638f8 input=95ca2d682c1938f6]*/
// clang-format on
{
  return self->items().release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.keys

Return an iterator over the keys in this ``ImmutableDict``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_keys_impl(pyimmutable::ImmutableDict::Wrapper*self)
/*[clinic end generated code: output=e298b2af41db2ba4 input=6ee4186b13a38107]*/
// clang-format on
{
  return self->keys().release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.pop

  key: object
  /

Return a copy with ``key`` removed.

Raises ``KeyError`` if ``key`` is not present.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_pop(pyimmutable::ImmutableDict::Wrapper*self,
                               PyObject *key)
/*[clinic end generated code: output=9a33828a75be1663 input=89e6177b8b3b7eb9]*/
// clang-format on
{
  return self->discard<true>(key).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.set

  key: object
  value: object
  /

Return a copy with ``key`` set to ``value``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_set_impl(pyimmutable::ImmutableDict::Wrapper*self,
                                    PyObject *key, PyObject *value)
/*[clinic end generated code: output=74f7aaabb0dbe898 input=5bdbd57d2bf275df]*/
// clang-format on
{
  return self->set(key, value).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableDict.values

Return an iterator over the values in this ``ImmutableDict``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableDict_values_impl(pyimmutable::ImmutableDict::Wrapper*self)
/*[clinic end generated code: output=5d686debdb9abe8c input=58a9e1a9ead998ea]*/
// clang-format on
{
  return self->values().release();
}

//////////////////////////////////////////////////////////////////////////////

using namespace pyimmutable;

namespace {

// clang-format off
PyMethodDef ImmutableDict_methods[] = {
    _PYIMMUTABLE_IMMUTABLEDICT__GET_INSTANCE_COUNT_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_DISCARD_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_GET_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_ITEMS_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_KEYS_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_POP_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_SET_METHODDEF
    _PYIMMUTABLE_IMMUTABLEDICT_VALUES_METHODDEF
    {"update",
     reinterpret_cast<PyCFunction>(static_cast<PyCFunctionWithKeywords>(
         ImmutableDict::Wrapper::method<&ImmutableDict::update>())),
     METH_VARARGS | METH_KEYWORDS,
     docstring_ImmutableDict_update
   },
    {nullptr}};
// clang-format on

PyMappingMethods ImmutableDict_mappingMethods = {
    .mp_length = ImmutableDict::Wrapper::method<&ImmutableDict::len>(),
    .mp_subscript = ImmutableDict::Wrapper::method<&ImmutableDict::getItem>(),
    .mp_ass_subscript = nullptr,
};

PyGetSetDef ImmutableDict_getset[] = {
    {"isImmutableJson",
     ImmutableDict::Wrapper::method<&ImmutableDict::isImmutableJsonDict>(),
     nullptr,
     docstring_ImmutableDict_isImmutableJson,
     nullptr},
    {"meta",
     ImmutableDict::Wrapper::method<&ImmutableDict::meta>(),
     nullptr,
     docstring_ImmutableDict_meta,
     nullptr},
    {nullptr}};

} // namespace

namespace pyimmutable {
bool isImmutableJsonDict(PyObject* obj) {
  return ImmutableDict::Wrapper::cast(obj)->isImmutableJson;
}

template <>
PyTypeObject ImmutableDict::Wrapper::typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableDict",
    .tp_repr = ImmutableDict::Wrapper::method<&ImmutableDict::repr>(),
    .tp_as_mapping = &ImmutableDict_mappingMethods,
    .tp_doc = docstring_ImmutableDict,
    .tp_iter = ImmutableDict::Wrapper::method<&ImmutableDict::keys>(),
    .tp_methods = ImmutableDict_methods,
    .tp_getset = ImmutableDict_getset,
    .tp_new = mangleReturnValue<&ImmutableDict::new_>(),
};
PyTypeObject* getImmutableDictTypeObject() {
  return ImmutableDict::Wrapper::initType();
}
template <>
detail::Sha1Lookup<ImmutableDict>::LookUpMapType*
    detail::Sha1Lookup<ImmutableDict>::lookUpMap_{nullptr};

template <>
PyTypeObject ImmutableDictIter::Wrapper::typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableDictIterator",
    .tp_iter = [](PyObject* self) { return PyObjectRef{self}.release(); },
    .tp_iternext =
        ImmutableDictIter::Wrapper::method<&ImmutableDictIter::next>(),
    .tp_new = &disallow_construction,
};
PyTypeObject* getImmutableDictIterTypeObject() {
  return ImmutableDictIter::Wrapper::initType();
}

} // namespace pyimmutable
