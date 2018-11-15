#include <Python.h>

namespace pyimmutable {

namespace detail {
template <typename T>
struct IsPyObject {
  static constexpr bool value = offsetof(T, ob_base) == 0;
};

template <>
struct IsPyObject<PyObject> : std::true_type {};
} // namespace detail

template <typename T = PyObject>
class TypedPyObjectRef final {
  static_assert(detail::IsPyObject<T>::value);

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

  T* getPyObject() const noexcept {
    return reinterpret_cast<PyObject*>(ptr_);
  }

  PyObject* release() noexcept {
    return reinterpret_cast<PyObject*>(std::exchange(ptr_, nullptr));
  }

  T* operator->() const {
    return get();
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
