@echo off

title Build ANUBiS under Windows.
echo Starting Build Process...
mkdir build
cd build
cmake .. -DCMAKE_SH="CMAKE_SH-NOTFOUND" -G "MinGW Makefiles" -DBOOST_WIN32_COMPILER="gcc"
mingw32-make SHELL=cmd
cd ..