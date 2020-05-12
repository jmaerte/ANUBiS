#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
mkdir build
cd build
cmake .. -DARCHITECTURE=64 -DCMAKE_BUILD_TYPE=Debug -DBOOST_PATH=/home/jmaerte/Downloads/boost_1_72_0.tar.bz2
make
cd ..
