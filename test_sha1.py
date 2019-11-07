"""
This is a simple stand-alone script to test the SHA1 algorithm in cpp/Sha1.h
"""

import hashlib
import subprocess


def main():
    with open("Sha1Test.cpp", "w") as f:
        TestWriter(f).write()

    subprocess.check_call(
        [
            "g++",
            "-O2",
            "Sha1Test.cpp",
            "-o",
            "Sha1Test",
            "-Wall",
            "-Wextra",
            "-std=c++17",
        ]
    )
    subprocess.check_call(["bash", "-c", "time ./Sha1Test"])


class TestWriter:
    def __init__(self, out):
        self.__out = out
        self.__count = 0

        # Create some data to work with
        data = self.__data = bytearray(10000)
        for i in range(0, 10000, 20):
            data[i : i + 20] = hashlib.sha1(data[0:i]).digest()

    def write(self):
        self.__out.write(self.header)
        self.add_assert([(0, 10000)])

        for i in range(200):
            self.add_assert([(0, i)])

        for i in range(50, 70):
            for j in range(50, 70):
                self.add_assert([(i * 10, i), (1000 - j, j)])

        digest = self.__format_digest(hashlib.sha1(self.__data[0:1000]))
        for i in (0, 1, 2, 55, 56, 57, 63, 64, 65, 66, 333):
            for j in (0, 1, 2, 55, 56, 57, 63, 64, 65, 66, 333):
                for k in (0, 1, 2, 55, 56, 57, 63, 64, 65, 66, 333):
                    self.add_assert(
                        [
                            (0, i),
                            (i, j),
                            (i + j, k),
                            (i + j + k, 1000 - i - j - k),
                        ],
                        digest=digest,
                    )

        self.__out.write("for(int X=0; X<100000; ++X) {\n")
        self.add_assert([(0, 10000)])
        self.__out.write("}\n")

        self.__out.write(self.footer)

    def add_assert(self, ranges, *, digest=None):
        out = self.__out
        data = self.__data

        self.__count += 1
        number = self.__count

        if digest is None:
            h = hashlib.sha1()
            for begin, length in ranges:
                h.update(data[begin : begin + length])
            digest = self.__format_digest(h)

        out.write(f"// Test #{ number }\nif (Context()")
        for begin, length in ranges:
            out.write(f"(data+{begin}, {length})")
        out.write(f".final() != { digest })\n")
        out.write(
            f'{{ std::cerr << "Test #{ number } failed!" << std::endl; '
            "return 1; }\n\n"
        )

    @staticmethod
    def __format_digest(h):
        return "Hash{" + ",".join(map(str, h.digest())) + "}"

    header = """\
#include "cpp/Sha1.h"
#include <iostream>

using namespace sha1;

int main()
{
    uint8_t data[10000];

    for (int i=0; i<10000; i+=20) {
        auto h = Context()(data, i).final();
        std::copy(h.begin(), h.end(), data+i);
    }

"""

    footer = """
    std::cerr << "Finished successfully." << std::endl;

    return 0;
}
"""


if __name__ == "__main__":
    main()
