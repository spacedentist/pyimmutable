from setuptools import setup, Extension

setup(
    name='pyimmutable',
    version='0.1',
    description='immutable dict and lists',
    url='https://github.com/spacedentist/pyimmutable',
    author='Sven Over',
    author_email='sp@cedenti.st',
    license='MIT',
    packages=['pyimmutable'],
    ext_modules=[
        Extension(
            '_pyimmutable',
            [
                'cpp/ImmutableDict.cpp',
                'cpp/ImmutableList.cpp',
                'cpp/main.cpp',
                'cpp/util.cpp',
            ],
            language="c++",
            include_dirs=['include'],
            libraries=['ssl'],
            extra_compile_args=['-std=c++17'],
        ),
    ],
)
