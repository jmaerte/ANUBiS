# ANUBiS

* **A**ssemblage of **N**umerous **U**tilities for **Bi**g **S**implicial Data Structures

ANUBiS is a C++ library of high-performance algorithms for computing topological invariants of point sets and simplicial topological spaces. It can be used as a tool for topological data analysis or plotting computer generated triangulations of given surfaces.

## Why: Functionality and Motivation

The motivation consists in the will for a as flexible as possible toolbox for analyzing the behavior of simplicial topological spaces under certain transformations. Further we want to be able to simplify large complexes to such a degree that they are actually calculable. The main focus is to use state-of-the-art data structures to tackle this goal. Further there are some elaborate algorithms involved to speed up this computations as far as possible.

### Binding with Python

I tried to make porting this library to python as easy as possible in order to use it in a more flexible environment. The linkage to a C++ interface preserves the execution speed while offering a more easy to use interface to the user. Boost.Python or ctypes library from python standard library

## Usage

A full documentation can be found under

## Installation

ANUBiS comes with precompiled binaries for Ubuntu and Windows.

### Building it yourself

To build ANUBiS you just need to clone this repository. Since this is a CMake Project you only need to run

```
git clone https://github.com/jmaerte/ANUBiS.git
cd ANUBiS
mkdir build
cmake --build <path/to/project>/build --target ANUBiS -- -j 4
cd build
make
```

Remember to prepend `sudo` when necessary! The Shell command is necessary if you are on windows and have a sh.exe in your PATH environment variable. You should set it to cmd

Be aware that when building with MinGW on Windows you should not have a sh.exe in your PATH environment variable (this is due to MinGW). This can happen when you have installed git on your machine. Here are some Solutions and cases in which they function:

**If you are permitted to change system variables:**
Remove the path containing sh.exe from the PATH variable.

**You are building the project in the console:**
Append `SHELL=cmd` to the make command.

**You are using an IDE that does not let you set flags for the make command:**
Set the user environment variable `MAKEFLAGS=SHELL=cmd`.

## License

This Project is licensed under the terms of the MIT License.

