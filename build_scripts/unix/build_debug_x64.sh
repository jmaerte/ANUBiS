#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
sudo rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DARCHITECTURE=64
make
cd ..
