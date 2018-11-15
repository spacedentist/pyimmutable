#include <openssl/evp.h>

namespace pyimmutable {

inline constexpr std::size_t kHashSize = 20;
using Sha1Hash = std::array<unsigned char, kHashSize>;

class Sha1Hasher {
 public:
  Sha1Hasher() : ctx_(EVP_MD_CTX_new()) {
    EVP_DigestInit_ex(ctx_, EVP_sha1(), nullptr);
  }

  ~Sha1Hasher() {
    EVP_MD_CTX_free(ctx_);
  }

  Sha1Hasher(Sha1Hasher const& other) : Sha1Hasher() {
    EVP_MD_CTX_copy_ex(ctx_, other.ctx_);
  }

  Sha1Hash final() {
    Sha1Hash result;
    EVP_DigestFinal_ex(ctx_, result.data(), nullptr);
    return result;
  }
  Sha1Hasher& operator()(void const* data, std::size_t len) {
    EVP_DigestUpdate(ctx_, data, len);
    return *this;
  }
  Sha1Hasher& operator()(PyObject* obj) {
    if (PyUnicode_Check(obj)) {
      Py_ssize_t len = PyUnicode_GET_LENGTH(obj) * PyUnicode_KIND(obj);
      return (*this)("unc", 3)(static_cast<void const*>(&len), sizeof(len))(
          PyUnicode_DATA(obj), len);
    } else if (PyLong_Check(obj)) {
      PyLongObject* const lobj = reinterpret_cast<PyLongObject*>(obj);

      Py_ssize_t l = Py_SIZE(lobj);
      (*this)("lon", 3)(&l, sizeof(l));
      if (l > 0) {
        (*this)(lobj->ob_digit, l * sizeof(lobj->ob_digit[0]));
      } else if (l < 0) {
        (*this)(lobj->ob_digit, -l * sizeof(lobj->ob_digit[0]));
      }
      return (*this);
    } else if (PyFloat_Check(obj)) {
      double d = PyFloat_AS_DOUBLE(obj);
      return (*this)("flt", 3)(&d, sizeof(d));
    } else {
      return (*this)("obj", 3)(&obj, sizeof(PyObject*));
    }
  }

 private:
  EVP_MD_CTX* ctx_;
};

struct Sha1HashHasher {
  std::size_t operator()(Sha1Hash const& h) const {
    return *reinterpret_cast<std::size_t const*>(std::addressof(h));
  }
};

inline Sha1Hash&
xorHashInPlace(Sha1Hash& h1, Sha1Hash const& h2, unsigned int shift = 0) {
  for (std::size_t i = 0; i < kHashSize; ++i) {
    h1[i] ^= h2[(i + shift) % kHashSize];
  }

  return h1;
}

inline Sha1Hash
xorHash(Sha1Hash const& h1, Sha1Hash const& h2, unsigned int shift = 0) {
  Sha1Hash copy{h1};
  return xorHashInPlace(copy, h2, shift);
}

inline std::pair<Sha1Hash, Sha1Hash> keyValueHashes(
    PyObject* key,
    PyObject* value) {
  Sha1Hasher hasher;
  hasher(key);
  Sha1Hasher value_hasher{hasher};
  value_hasher(value);
  return {hasher.final(), value_hasher.final()};
}

inline Sha1Hash valueHash(PyObject* value) {
  return Sha1Hasher{}(value).final();
}

inline Sha1Hash itemHash(Sha1Hash const& value_hash, std::size_t idx) {
  return Sha1Hasher{}("itm", 3)(&idx, sizeof(idx))(
             value_hash.data(), value_hash.size())
      .final();
}

} // namespace pyimmutable
