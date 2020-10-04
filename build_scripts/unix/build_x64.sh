#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
sudo rm -rf build
mkdir build
cd build
cmake .. -DARCHITECTURE=64 -DCMAKE_BUILD_TYPE=Release -DBOOST_PATH=/media/jmaerte/DATA2/boost/lib
make
cd ..
