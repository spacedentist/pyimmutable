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

#include "ImmutableListImpl.h"
#include "clinic/ImmutableList.cpp.h"
#include "docstrings.autogen.h"

// clang-format off
/*[clinic input]
module _pyimmutable
class _pyimmutable.ImmutableList "pyimmutable::ImmutableList::Wrapper*" "pyimmutable::immutableListTypeObject"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=b16d643ce356d8c2]*/
// clang-format on

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.__reversed__

Return a reverse iterator over the ``ImmutableList``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList___reversed___impl(pyimmutable::ImmutableList::Wrapper*self)
/*[clinic end generated code: output=fa12047ef068bd1e input=de8c73049d0a0c2a]*/
// clang-format on
{
  return self->iterImpl(/* reversed= */ true).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
@staticmethod
_pyimmutable.ImmutableList._get_instance_count

Return the number of ``ImmutableList`` objects currently in existence.

This is mainly useful for the ``ImmutableList`` test suite.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList__get_instance_count_impl()
/*[clinic end generated code: output=e92bd248dadeb452 input=1f4921432c929bd5]*/
// clang-format on
{
  return PyLong_FromSize_t(
      pyimmutable::ImmutableList::Wrapper::getInstanceCount());
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.append

  value: object
  /

Return a copy with the given ``value`` appended.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList_append(pyimmutable::ImmutableList::Wrapper*self,
                                  PyObject *value)
/*[clinic end generated code: output=4425aa537f38ab23 input=fea20c3dcce6e1c0]*/
// clang-format on
{
  return self->append(value).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.count

  value: object
  /

Return number of occurrences of ``value``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList_count(pyimmutable::ImmutableList::Wrapper*self,
                                 PyObject *value)
/*[clinic end generated code: output=5a7032a195a41b7e input=227578d2726fd874]*/
// clang-format on
{
  return self->count(value).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.extend

  iterable: object
  /

Return a copy extended by appending elements from ``iterable``.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList_extend(pyimmutable::ImmutableList::Wrapper*self,
                                  PyObject *iterable)
/*[clinic end generated code: output=971548392eacf921 input=39a19f465c41b22d]*/
// clang-format on
{
  return self->extend(iterable).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.index

  value: object
  start: slice_index(accept={int}) = 0
  stop: slice_index(accept={int}, c_default="PY_SSIZE_T_MAX") = sys.maxsize
  /

Return first index of ``value``.

Raises ``ValueError`` if ``value`` is not present.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList_index_impl(pyimmutable::ImmutableList::Wrapper*self,
                                      PyObject *value, Py_ssize_t start,
                                      Py_ssize_t stop)
/*[clinic end generated code: output=fcfe7d0636c70cb9 input=beec301344457788]*/
// clang-format on
{
  return self->index(value, start, stop).release();
}

//////////////////////////////////////////////////////////////////////////////
// clang-format off
/*[clinic input]
_pyimmutable.ImmutableList.set

  index: Py_ssize_t
  value: object
  /

Return a copy with item ``index`` set to ``value``.

Raises ``IndexError`` if ``index`` is outside the range of existing elements.
[clinic start generated code]*/

static PyObject *
_pyimmutable_ImmutableList_set_impl(pyimmutable::ImmutableList::Wrapper*self,
                                    Py_ssize_t index, PyObject *value)
/*[clinic end generated code: output=34a51ab604f23326 input=aa292f72b6c2836b]*/
// clang-format on
{
  return self->set(index, value).release();
}

//////////////////////////////////////////////////////////////////////////////

using namespace pyimmutable;

namespace {

// clang-format off
PyMethodDef ImmutableList_methods[] = {
    _PYIMMUTABLE_IMMUTABLELIST___REVERSED___METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST__GET_INSTANCE_COUNT_METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST_APPEND_METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST_COUNT_METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST_EXTEND_METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST_INDEX_METHODDEF
    _PYIMMUTABLE_IMMUTABLELIST_SET_METHODDEF
    {nullptr}};
// clang-format on

PySequenceMethods ImmutableList_sequenceMethods = {
    .sq_length = ImmutableList::Wrapper::method<&ImmutableList::len>(),
    .sq_concat = ImmutableList::Wrapper::method<&ImmutableList::concat>(),
    .sq_repeat = ImmutableList::Wrapper::method<&ImmutableList::repeat>(),
    .sq_item = ImmutableList::Wrapper::method<&ImmutableList::getItemIdx>(),
    .sq_contains = ImmutableList::Wrapper::method<&ImmutableList::contains>(),
};

PyMappingMethods ImmutableList_mappingMethods = {
    .mp_length = ImmutableList::Wrapper::method<&ImmutableList::len>(),
    .mp_subscript = ImmutableList::Wrapper::method<&ImmutableList::getItem>(),
    .mp_ass_subscript = nullptr,
};

PyGetSetDef ImmutableList_getset[] = {
    {"isImmutableJson",
     ImmutableList::Wrapper::method<&ImmutableList::isImmutableJsonList>(),
     nullptr,
     docstring_ImmutableList_isImmutableJson,
     nullptr},
    {"meta",
     ImmutableList::Wrapper::method<&ImmutableList::meta>(),
     nullptr,
     docstring_ImmutableList_meta,
     nullptr},
    {nullptr}};

} // namespace

namespace pyimmutable {

bool isImmutableJsonList(PyObject* obj) {
  return ImmutableList::Wrapper::cast(obj)->isImmutableJson;
}

template <>
PyTypeObject ImmutableList::Wrapper::typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableList",
    .tp_repr = ImmutableList::Wrapper::method<&ImmutableList::repr>(),
    .tp_as_sequence = &ImmutableList_sequenceMethods,
    .tp_as_mapping = &ImmutableList_mappingMethods,
    .tp_doc = docstring_ImmutableList,
    .tp_iter = ImmutableList::Wrapper::method<&ImmutableList::iter>(),
    .tp_methods = ImmutableList_methods,
    .tp_getset = ImmutableList_getset,
    .tp_new = mangleReturnValue<&ImmutableList::new_>(),
};
PyTypeObject* getImmutableListTypeObject() {
  return ImmutableList::Wrapper::initType();
}
template <>
detail::Sha1Lookup<ImmutableList>::LookUpMapType*
    detail::Sha1Lookup<ImmutableList>::lookUpMap_{nullptr};

template <>
PyTypeObject ImmutableListIter::Wrapper::typeObject = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
        .tp_name = "ImmutableListIterator",
    .tp_iter = [](PyObject* self) { return PyObjectRef{self}.release(); },
    .tp_iternext =
        ImmutableListIter::Wrapper::method<&ImmutableListIter::next>(),
    .tp_new = &disallow_construction,
};
PyTypeObject* getImmutableListIterTypeObject() {
  return ImmutableListIter::Wrapper::initType();
}

} // namespace pyimmutable
