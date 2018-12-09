---
id: Build
title: Building and Installation
---

### Table of Contents
- [Building](#Building)
- [Installation](#Installation)
- [Compiler Support](#Compiler-Support)

## Building
This is a very small library, it is very easy to build and configure and requires no third-party support.
To build, use CMake to generate the system build files required for your platform. On...
- Unix Systems: The command is `make` which produce a release and debug version depending on the `CMAKE_BUILD_TYPE` specified.
Clang users may specify `USE_LIBCXX` to use _libc++_ instead of _stdlibc++_.
- Windows Systems: The usual MSVC files can be build through the IDE or via command line interface.
The examples require a C++1z compiler but the library is compatable with [older compilers](https://travis-ci.org/prince-chrismc/Simple-Socket).
However this, maintaining backwards compatability, is not a goal and may change at any time.

## Installation
Installation can be achieved by adding the runtime ( simply the .dll generate with the cmake BUILD_SHARED_LIBS as ON ) to either the execution
directory of your application or to a location of [PATH](http://www.linfo.org/path_env_var.html) variable.

## Compiler Support
Currently Tested:
- Clang 7.0
- GCC 7.3
- GCC 8.2
- MCVS 15.8
- Xcode 10.0
- Xcode 9.4

Formerly Tested:
- Clang 6.0
- MCVS 15.7
- Xcode 8.3
