@echo off

title Build ANUBiS under Windows.
echo Starting Build Process...
mkdir build && cd build
cmake ..
make SHELL=cmd