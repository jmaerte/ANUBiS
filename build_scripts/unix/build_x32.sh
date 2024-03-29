#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
mkdir build
cd build
cmake .. -DARCHITECTURE=32 -DCMAKE_BUILD_TYPE=Release
make
cd ..