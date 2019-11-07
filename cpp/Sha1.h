/*
 * PUBLIC DOMAIN
 *
 * C++17 SHA1 implementation by Sven Over <sp@cedenti.st>.
 * Based on the Wikipedia article about SHA1 and on
 * Steve Reid's <steve@edmweb.com> public domain implementation.
 *
 * This is 100% public domain.
 *
 */

#include <array>
#include <cstdint>
#include <endian.h>
#include <utility>

namespace sha1 {

inline constexpr std::size_t kHashSize = 20;
using Hash = std::array<uint8_t, kHashSize>;

class alignas(16) Context {
 public:
  Context& operator()(void const* data, std::size_t len) {
    uint8_t const* ptr = reinterpret_cast<uint8_t const*>(data);

    while (len) {
      unsigned int const bytes_in_buffer = count % 64;
      unsigned int const copy_bytes =
          std::min<unsigned int>(64 - bytes_in_buffer, len);
      std::copy(ptr, ptr + copy_bytes, buffer.bytes + bytes_in_buffer);
      ptr += copy_bytes;
      len -= copy_bytes;
      count += copy_bytes;

      if (count % 64 == 0) {
        // buffer is full
        process_buffer();
      }
    }
    return *this;
  }

  Hash final() {
    uint64_t const message_length_in_bits = count * 8;

    buffer.bytes[count++ % 64] = 0x80;
    if (count % 64 == 0) {
      // buffer is full
      process_buffer();
    }

    unsigned int bytes_in_buffer = count % 64;
    if (bytes_in_buffer > 56) {
      std::fill(buffer.bytes + bytes_in_buffer, buffer.bytes + 64, 0);
      process_buffer();
      bytes_in_buffer = 0;
    }

    std::fill(buffer.bytes + bytes_in_buffer, buffer.bytes + 56, 0);

    *reinterpret_cast<uint64_t*>(&buffer.bytes[56]) =
        ::htobe64(message_length_in_bits);
    process_buffer();

    Hash result;
    for (int i = 0; i < 20; ++i) {
      result[i] = state[i / 4] >> ((3 - (i % 4)) * 8);
    }
    return result;
  }

 private:
  static uint32_t leftrotate(uint32_t value, unsigned int bits) {
    return ((value << bits) | (value >> (32 - bits)));
  }

  template <std::size_t I>
  void doStep() {
    uint32_t& a = state[(80 - I) % 5];
    uint32_t& b = state[(81 - I) % 5];
    uint32_t& c = state[(82 - I) % 5];
    uint32_t& d = state[(83 - I) % 5];
    uint32_t& e = state[(84 - I) % 5];

    if constexpr (I < 16) {
      buffer.words[I] = ::be32toh(buffer.words[I]);
    } else {
      buffer.words[I % 16] = leftrotate(
          (buffer.words[(I - 3) % 16] ^ buffer.words[(I - 8) % 16] ^
           buffer.words[(I - 14) % 16] ^ buffer.words[I % 16]),
          1);
    }

    e += buffer.words[I % 16] + leftrotate(a, 5);

    if constexpr (I < 20) {
      e += ((b & (c ^ d)) ^ d) + 0x5A827999;
    } else if constexpr (I < 40) {
      e += (b ^ c ^ d) + 0x6ED9EBA1;
    } else if constexpr (I < 60) {
      e += (((b | c) & d) | (b & c)) + 0x8F1BBCDC;
    } else {
      e += (b ^ c ^ d) + 0xCA62C1D6;
    }

    b = leftrotate(b, 30);
  }

  template <std::size_t... I>
  void doSteps(std::index_sequence<I...>) {
    (void)std::array<int, 81>{0, (doStep<I>(), 0)...};
  }

  void process_buffer() {
    auto const saved_state = state;

    doSteps(std::make_index_sequence<80>());

    state[0] += saved_state[0];
    state[1] += saved_state[1];
    state[2] += saved_state[2];
    state[3] += saved_state[3];
    state[4] += saved_state[4];
  }

  union {
    uint8_t bytes[64];
    uint32_t words[16];
  } buffer;
  std::array<uint32_t, 6> state = {0x67452301,
                                   0xEFCDAB89,
                                   0x98BADCFE,
                                   0x10325476,
                                   0xC3D2E1F0};
  uint64_t count = 0;
};

} // namespace sha1
