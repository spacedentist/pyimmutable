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

#include "Sha1.h"

namespace pyimmutable {

using Sha1Hash = sha1::Hash;

struct Sha1HashHasher {
  std::size_t operator()(Sha1Hash const& h) const {
    return *reinterpret_cast<std::size_t const*>(std::addressof(h));
  }
};

inline Sha1Hash&
xorHashInPlace(Sha1Hash& h1, Sha1Hash const& h2, unsigned int shift = 0) {
  for (std::size_t i = 0; i < sha1::kHashSize; ++i) {
    h1[i] ^= h2[(i + shift) % sha1::kHashSize];
  }

  return h1;
}

inline Sha1Hash
xorHash(Sha1Hash const& h1, Sha1Hash const& h2, unsigned int shift = 0) {
  Sha1Hash copy{h1};
  return xorHashInPlace(copy, h2, shift);
}

} // namespace pyimmutable
