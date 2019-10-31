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

#include <Python.h>

namespace pyimmutable {
namespace {

template <typename C, auto M>
auto method() {
  return [](PyObject* pyself, auto... args) {
    C* const self = reinterpret_cast<C*>(pyself);
    return (self->*M)(std::move(args)...);
  };
}

template <typename F>
class OnDestroy {
 public:
  template <
      typename T,
      typename = std::enable_if_t<std::is_invocable_v<std::decay_t<T>>>>
  OnDestroy(T&& f) : func_(std::forward<T>(f)) {}
  ~OnDestroy() {
    std::move(func_)();
  }

  OnDestroy(OnDestroy const&) = delete;
  OnDestroy& operator=(OnDestroy const&) = delete;

 private:
  F func_;
};

template <typename F>
OnDestroy(F &&)->OnDestroy<std::decay_t<F>>;

} // namespace

bool isImmutableJsonObject(PyObject*);

PyObject* disallow_construction(
    PyTypeObject* /*type*/,
    PyObject* /*args*/,
    PyObject*
    /*kwds*/);

} // namespace pyimmutable
