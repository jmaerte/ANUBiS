#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
mkdir build && cd build
cmake ..
make