from setuptools import setup, Extension

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="pyimmutable",
    version="0.1.3",
    description="Immutable dict and lists",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/spacedentist/pyimmutable",
    download_url=(
        "https://github.com/spacedentist/pyimmutable/archive/v0.1.3.tar.gz"
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
)
