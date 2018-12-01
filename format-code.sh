#!/bin/bash -e

shopt -s globstar

# Format all Python code
black *.py pyimmutable/**/*.py

# Format C++ code
clang-format -i cpp/**/*.{cpp,h}

# Run flake8
flake8 *.py pyimmutable/**/*.py
