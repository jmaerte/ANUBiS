@echo off

title Build ANUBiS under Windows.
echo Starting Build Process...
mkdir build && cd build
cmake .. -DCMAKE_SH="CMAKE_SH-NOTFOUND" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DARCHITECTURE=32
make SHELL=cmd