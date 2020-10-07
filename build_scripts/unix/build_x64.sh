#!/bin/bash
# Build ANUBiS on a Unix System!

echo Starting Build Process...
#sudo rm -rf build
if [ -d "build" ]; then
  sudo rm -rf build/ANUBiS build/ANUBiS-prefix
fi
mkdir build
cd build
cmake .. -DARCHITECTURE=64 -DCMAKE_BUILD_TYPE=Release -DBOOST_PATH=/media/jmaerte/DATA2/boost/lib
make
cd ..
