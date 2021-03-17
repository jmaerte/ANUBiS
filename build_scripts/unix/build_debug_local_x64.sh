#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
sudo rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DARCHITECTURE=64 -DBOOST_PATH="/media/jmaerte/DATA2/Boost_1_72_0/boost_1_72_0.tar.bz2"
make
cd ..
