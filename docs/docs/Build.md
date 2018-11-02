---
id: Build
title: Building and Installation
---
This is a very small library, it is very easy to build and configure and requires no third-party support.
To build and install, use CMake to generate the files required for your platform and execute the appropriate build command or procedure.

- Unix Systems: The command is `make` which produce a release and debug version depending on the `CMAKE_BUILD_TYPE` specified.
- Windows Systems: The usual MSVC files can be build through the IDE or via command line interface.

Installation can be achieved by adding the runtime ( simply the .dll generate with the cmake SIMPLE_SOCKET_SHARED as ON ) to either the execution
directory of your application or to a location of [PATH](http://www.linfo.org/path_env_var.html) variable.

Currently Tested:
- Clang 6.0
- Clang 7
- GCC 7.3
- GCC 8.1
- MCVS 15.7
- MCVS 15.8