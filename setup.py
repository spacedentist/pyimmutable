from setuptools import setup, Extension
import setuptools.command.build_ext


class BuildExt(setuptools.command.build_ext.build_ext):
    def build_extension(self, *args, **kwargs):
        compile_docstrings()
        return super().build_extension(*args, **kwargs)


def compile_docstrings():
    with open("cpp/docstrings.txt") as input, open(
        "cpp/docstrings.autogen.h", "w"
    ) as output:
        key = None
        text = ""

        def write():
            if key:
                stripped_text = "\n".join(
                    line.rstrip() for line in text.rstrip().split("\n")
                )
                output.write(f"static constexpr char const* {key} = ")
                delim = "TEXT"
                x = 0
                while f"){ delim }" in stripped_text:
                    delim = f"TEXT{x}"
                    x += 1
                output.write(f'R"{ delim }({ stripped_text }){ delim }";\n')

        for line in input:
            if line.startswith("<@>"):
                write()
                key = line[3:].strip()
                text = ""
            else:
                text += line
        write()


with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="pyimmutable",
    version="0.1.4",
    description="Immutable dict and lists",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/spacedentist/pyimmutable",
    download_url=(
        "https://github.com/spacedentist/pyimmutable/archive/v0.1.4.tar.gz"
    ),
    author="Sven Over",
    author_email="sp@cedenti.st",
    license="MIT",
    packages=["pyimmutable"],
    ext_modules=[
        Extension(
            "_pyimmutable",
            sources=[
                "cpp/ImmutableDict.cpp",
                "cpp/ImmutableList.cpp",
                "cpp/main.cpp",
                "cpp/util.cpp",
            ],
            depends=[
                "cpp/Hash.h",
                "cpp/ImmutableDict.h",
                "cpp/ImmutableList.h",
                "cpp/PyObjectRef.h",
                "cpp/util.h",
                "cpp/docstrings.txt",
            ],
            language="c++",
            include_dirs=["lib/immer"],
            libraries=["ssl", "crypto"],
            extra_compile_args=["-std=c++17"],
        )
    ],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    test_suite="pyimmutable.tests",
    setup_requires=["tox"],
    cmdclass={"build_ext": BuildExt},
)
