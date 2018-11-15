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

} // namespace
} // namespace pyimmutable
