#!/usr/bin/env bash

# Fetch git submodules.
git submodule update --init

# Compile minimap2.
cd ./minimap2
make
cd ..

# TODO Compile HERA project.
# ...
