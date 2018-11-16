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

} // namespace pyimmutable
