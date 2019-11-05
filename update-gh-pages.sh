#!/bin/bash -e

# Get the commit hash for which we are building docs
head="$(git rev-parse HEAD)"

# Clean-up docs directory
git clean -fdx docs/

# Install current version
python setup.py install

# Create HTML docs
make -C docs html

# Add crucial .nojekyll file
touch docs/_build/html/.nojekyll

# Now add the docs to the git index
git add -f docs/_build/html

# Create a git tree object of the index, which includes the html docs
tree="$(git write-tree)"

# Clear the index
git reset --mixed HEAD

# Clean-up docs directory
git clean -fdx docs/

# Create new commit
commit="$(
  git commit-tree -p refs/heads/gh-pages -p "$head" \
    -m "Documentation for ${head}" \
    "${tree}:docs/_build/html" </dev/null)"

# Update gh-pages branch
git update-ref refs/heads/gh-pages "$commit"
