#!/bin/bash -e

shopt -s globstar

# Format all Python code
black --line-length 79 *.py pyimmutable/**/*.py

# Run flake8
flake8 *.py pyimmutable/**/*.py

# Format C++ code
clang-format -i cpp/*.{cpp,h}
