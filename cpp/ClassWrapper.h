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

#include <exception>
#include <functional>
#include <type_traits>

#include "PyObjectRef.h"
#include "Sha1Hash.h"
#include "util.h"

namespace pyimmutable {
template <typename T>
class ClassWrapper;

namespace detail {

template <typename T>
struct WeakRefHelper {
  static std::false_type func(void*);

  template <typename U>
  static std::bool_constant<U::weakrefs_enabled> func(U*);

  constexpr static bool value = decltype(func(std::declval<T*>()))::value;
};

template <typename T>
struct Sha1LookUpHelper {
  static std::false_type func(void*);

  template <typename U>
  static std::bool_constant<U::sha1_lookup_enabled> func(U*);

  constexpr static bool value = decltype(func(std::declval<T*>()))::value;
};

template <bool>
struct WeakRefs {
  static constexpr bool weakrefs_enabled = false;
};

template <>
struct WeakRefs<true> {
  static constexpr bool weakrefs_enabled = true;
  PyObject* weakreflist_;
};

template <typename T>
class Sha1Lookup {
 protected:
  static constexpr bool sha1_lookup_enabled = true;

 public:
  using LookUpMapType =
      std::unordered_map<Sha1Hash, ClassWrapper<T>*, Sha1HashHasher>;

  template <typename Factory>
  static TypedPyObjectRef<ClassWrapper<T>> getOrCreate(
      Sha1Hash const& hash,
      Factory&& f);

  static std::size_t getInstanceCount() {
    if (lookUpMap_) {
      return lookUpMap_->size();
    } else {
      return 0;
    }
  }

 private:
  static LookUpMapType* lookUpMap_;

 protected:
  static void init() {
    delete lookUpMap_;
    lookUpMap_ = new LookUpMapType;
  }
  static void shutdown() {
    delete lookUpMap_;
  }
  static void destroy(ClassWrapper<T>* self) {
    if (lookUpMap_) {
      lookUpMap_->erase(self->sha1);
    }
  }
};

template <>
class Sha1Lookup<void> {
 protected:
  static constexpr bool sha1_lookup_enabled = false;
  static std::size_t getInstanceCount() {
    return 0;
  }
};

struct PyObjectHead {
  PyObject_HEAD;

  PyObject* ptr() {
    return reinterpret_cast<PyObject*>(this);
  }

  static PyObjectHead* cast(PyObject* ptr) {
    return reinterpret_cast<PyObjectHead*>(ptr);
  }
};
static_assert(offsetof(PyObjectHead, ob_base) == 0);

template <typename T>
T&& mangleReturnValue(T&& t) {
  return static_cast<T&&>(t);
}

inline PyObject* mangleReturnValue(PyObjectRef obj) {
  return obj.release();
}

} // namespace detail

template <auto F>
auto mangleReturnValue() {
  return [](auto... args) {
    return detail::mangleReturnValue(std::invoke(F, std::move(args)...));
  };
}

template <typename T>
class ClassWrapper
    : private detail::PyObjectHead,
      public T,
      private detail::WeakRefs<detail::WeakRefHelper<T>::value>,
      public detail::Sha1Lookup<
          std::conditional_t<detail::Sha1LookUpHelper<T>::value, T, void>> {
 public:
  static PyTypeObject typeObject;

 private:
  bool objectConstructed_;

  using Sha1Lookup = detail::Sha1Lookup<
      std::conditional_t<detail::Sha1LookUpHelper<T>::value, T, void>>;
  using Sha1Lookup::sha1_lookup_enabled;
  using detail::WeakRefs<detail::WeakRefHelper<T>::value>::weakrefs_enabled;

  ClassWrapper() = delete;
  ClassWrapper(ClassWrapper const&) = delete;
  ClassWrapper& operator=(ClassWrapper const&) = delete;
  ~ClassWrapper() = delete;

  static void destroy(PyObject* pyself) {
    auto* self = cast(pyself);
    if constexpr (sha1_lookup_enabled) {
      Sha1Lookup::destroy(self);
    }

    if constexpr (weakrefs_enabled) {
      if (self->weakreflist_) {
        PyObject_ClearWeakRefs(pyself);
      }
    }

    if (self->objectConstructed_) {
      static_cast<T*>(self)->~T();
      self->objectConstructed_ = false;
    }

    PyObject_Del(pyself);
  }

 public:
  using detail::PyObjectHead::ptr;

  template <typename U>
  static ClassWrapper* cast(U* ptr) {
    return static_cast<ClassWrapper*>(static_cast<T*>(ptr));
  }
  static ClassWrapper* cast(PyObject* ptr) {
    return static_cast<ClassWrapper*>(PyObjectHead::cast(ptr));
  }
  template <typename U>
  static PyObject* pyObject(U* ptr) {
    return cast(ptr)->PyObjectHead::ptr();
  }

  template <typename... Args>
  static TypedPyObjectRef<ClassWrapper> create(Args&&... args) {
    TypedPyObjectRef<ClassWrapper> cw{PyObject_New(ClassWrapper, &typeObject),
                                      false};

    if (cw) {
      cw->objectConstructed_ = false;
      if constexpr (weakrefs_enabled) {
        cw->weakreflist_ = nullptr;
      }

      T* const t_ptr = static_cast<T*>(cw.get());

      try {
        new (t_ptr) T(std::forward<Args>(args)...);
        cw->objectConstructed_ = true;
      } catch (std::exception const& ex) {
        cw = {};
        PyErr_SetString(PyExc_Exception, ex.what());
      }
    }

    return cw;
  }

  static PyTypeObject* initType(void (*func)(PyTypeObject*) = nullptr) {
    typeObject.tp_basicsize = sizeof(ClassWrapper);
    typeObject.tp_dealloc = &ClassWrapper::destroy;
    if constexpr (weakrefs_enabled) {
      typename std::aligned_union<0, ClassWrapper>::type const u{};
      ClassWrapper const* const cw = reinterpret_cast<ClassWrapper const*>(&u);

      typeObject.tp_weaklistoffset =
          reinterpret_cast<std::intptr_t>(std::addressof(cw->weakreflist_)) -
          reinterpret_cast<std::intptr_t>(cw);
    } else {
      typeObject.tp_weaklistoffset = 0;
    }
    if (func) {
      func(&typeObject);
    }

    if (PyType_Ready(&typeObject) < 0) {
      return nullptr;
    }

    if constexpr (sha1_lookup_enabled) {
      Sha1Lookup::init();
    }

    return &typeObject;
  }

  static void shutdown() {
    if constexpr (sha1_lookup_enabled) {
      Sha1Lookup::shutdown();
    }
  }

  template <auto M>
  static auto method() {
    return [](PyObject* pyself, auto... args) {
      return detail::mangleReturnValue(
          std::invoke(M, cast(pyself), std::move(args)...));
    };
  }

  using Sha1Lookup::getInstanceCount;
};

namespace detail {
template <typename T>
template <typename Factory>
TypedPyObjectRef<ClassWrapper<T>> Sha1Lookup<T>::getOrCreate(
    Sha1Hash const& hash,
    Factory&& f) {
  if (lookUpMap_) {
    auto it = lookUpMap_->find(hash);
    if (it != lookUpMap_->end()) {
      return TypedPyObjectRef{it->second};
    }
  }

  auto obj = ClassWrapper<T>::create(std::forward<Factory>(f)());
  if (lookUpMap_) {
    lookUpMap_->emplace(hash, obj.get());
  }

  return obj;
}
} // namespace detail

} // namespace pyimmutable
