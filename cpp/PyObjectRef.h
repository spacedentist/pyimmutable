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

#pragma once

#include <cstddef>
#include <utility>

#include <Python.h>

#include "util.h"

namespace pyimmutable {

template <typename T = PyObject>
class TypedPyObjectRef final {
  static_assert(IsPyObject<T>::value);

  template <typename>
  friend class TypedPyObjectRef;

 public:
  TypedPyObjectRef() noexcept = default;
  ~TypedPyObjectRef() noexcept {
    Py_XDECREF(ptr_);
  }

  TypedPyObjectRef(TypedPyObjectRef const& other) noexcept : ptr_(other.ptr_) {
    Py_XINCREF(ptr_);
  }

  TypedPyObjectRef(TypedPyObjectRef&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr)) {}

  TypedPyObjectRef& operator=(TypedPyObjectRef const& other) {
    if (ptr_ != other.ptr_) {
      *this = TypedPyObjectRef(other);
    }
    return *this;
  }

  TypedPyObjectRef& operator=(TypedPyObjectRef&& other) noexcept {
    Py_XDECREF(ptr_);
    ptr_ = std::exchange(other.ptr_, nullptr);
    return *this;
  }

  explicit TypedPyObjectRef(T* ptr, bool incref = true) noexcept : ptr_(ptr) {
    if (incref) {
      Py_XINCREF(ptr_);
    }
  }

  TypedPyObjectRef(std::nullptr_t const&) noexcept : TypedPyObjectRef() {}

  template <
      typename U = T,
      typename = std::enable_if_t<!std::is_same_v<U, PyObject>>>
  explicit TypedPyObjectRef(TypedPyObjectRef<PyObject>&& other) noexcept
      : ptr_(reinterpret_cast<T*>(std::exchange(other.ptr_, nullptr))) {}

  template <
      typename U,
      typename = std::enable_if_t<
          std::is_same_v<T, PyObject> && !std::is_same_v<U, PyObject>>>
  /* implicit */ TypedPyObjectRef(TypedPyObjectRef<U>&& other) noexcept
      : ptr_(reinterpret_cast<PyObject*>(std::exchange(other.ptr_, nullptr))) {}

  T* get() const noexcept {
    return ptr_;
  }

  PyObject* getPyObject() const noexcept {
    return reinterpret_cast<PyObject*>(ptr_);
  }

  PyObject* release() noexcept {
    return reinterpret_cast<PyObject*>(std::exchange(ptr_, nullptr));
  }

  T* operator->() const {
    return get();
  }

  T& operator*() const {
    return *get();
  }

  TypedPyObjectRef copy() const {
    return TypedPyObjectRef(*this);
  }

  explicit operator bool() const {
    return ptr_;
  }

  static TypedPyObjectRef create(PyTypeObject* type, Py_ssize_t s = 0) {
    return TypedPyObjectRef{PyObject_New(T, type), false};
  }

 private:
  T* ptr_{nullptr};
};

template <typename T>
TypedPyObjectRef(T*)->TypedPyObjectRef<T>;

using PyObjectRef = TypedPyObjectRef<>;

template <typename... Args>
PyObjectRef buildValue(Args&&... args) {
  return PyObjectRef{Py_BuildValue(std::forward<Args>(args)...), false};
}

inline auto none() {
  return PyObjectRef{Py_None};
}

} // namespace pyimmutable
