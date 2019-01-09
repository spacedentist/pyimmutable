from setuptools import setup, Extension

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name="pyimmutable",
    version="0.1.1",
    description="Immutable dict and lists",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/spacedentist/pyimmutable",
    author="Sven Over",
    author_email="sp@cedenti.st",
    license="MIT",
    packages=["pyimmutable"],
    ext_modules=[
        Extension(
            "_pyimmutable",
            [
                "cpp/ImmutableDict.cpp",
                "cpp/ImmutableList.cpp",
                "cpp/main.cpp",
                "cpp/util.cpp",
            ],
            language="c++",
            include_dirs=["lib/immer"],
            libraries=["ssl"],
            extra_compile_args=["-std=c++17"],
        )
    ],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
)
