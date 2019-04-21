cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Simple-Socket)

# CMake Settings
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

#set(CMAKE_VERBOSE_MAKEFILE ON)

# OS and compiler checks.
if(UNIX)
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXECUTABLE_SUFFIX .run)
    set(THREAD_LIB pthread)
    set(WARNING_FLAGS -Wall -Wextra)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
      add_compile_options(-g -D_DEBUG)
    elseif(CMAKE_BUILD_TYPE MATCHES Release)
      add_compile_options(-O3)
    else()
      set(CMAKE_BUILD_TYPE Release)
      message("Defaulting to 'Release' configuration.")
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_definitions(-D_CLANG)
        if(USE_LIBCXX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        endif()
    endif()

    if(APPLE)
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)
    set(PROJECT_LIBS Ws2_32.lib)
    set(WARNING_FLAGS /W4)

    if(MSVC)
        add_compile_options(/std:c++17)
    else()
        message( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

# Setup versioning.
set(SIMPLE_SOCKET_MAJOR_VERSION "2")
set(SIMPLE_SOCKET_MINOR_VERSION "0")
set(SIMPLE_SOCKET_PATCH_VERSION "0")
set(SIMPLE_SOCKET_VERSION
    ${SIMPLE_SOCKET_MAJOR_VERSION}.${SIMPLE_SOCKET_MINOR_VERSION}.${SIMPLE_SOCKET_PATCH_VERSION})

# Setup source files
include_directories(src)

set(DATA_FILE docs/README.md )
file(GLOB SIMPLE_SOCKET_HEADERS "src/*.h")
file(GLOB SIMPLE_SOCKET_SOURCES "src/*.cpp")
LIST(APPEND SIMPLE_SOCKET_SOURCES ${SIMPLE_SOCKET_HEADERS})

# Project Options
option(BUILD_SHARED_LIBS "Build Simple-Socket as a shared library." OFF)
option(USE_LIBCXX "Build using libc++ as the standard library when using clang." OFF)

option(SIMPLE_SOCKET_STRING_VIEW "Generate a version of Simple-Socket allowing for use for string view" ON)

option(SIMPLE_SOCKET_EXAMPLES "Build the examples" ON)
option(SIMPLE_SOCKET_TEST "Build the tester" OFF)
option(SIMPLE_SOCKET_COVERAGE "Build the tester for code coverage" OFF)
mark_as_advanced(SIMPLE_SOCKET_TEST SIMPLE_SOCKET_COVERAGE)

# Build the lib and install into configured prefix
add_library(Simple-Socket ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
target_compile_features(Simple-Socket PUBLIC cxx_std_11)
set_target_properties(Simple-Socket PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON COMPILE_FLAGS "${WARNING_FLAGS}")
if(SIMPLE_SOCKET_STRING_VIEW)
    target_compile_features(Simple-Socket PUBLIC cxx_std_17)
    set_target_properties(Simple-Socket PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON COMPILE_FLAGS "${WARNING_FLAGS}")
    target_compile_options(Simple-Socket INTERFACE -DSTRING_VIEW)
endif()
target_compile_options(Simple-Socket INTERFACE ${WARNING_FLAGS})
target_link_libraries(Simple-Socket ${PROJECT_LIBS})
set_target_properties(Simple-Socket PROPERTIES VERSION ${SIMPLE_SOCKET_VERSION} SOVERSION ${SIMPLE_SOCKET_MAJOR_VERSION})

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include/)
set(LIBRARY_INSTALL_DIR lib/)
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/Simple-Socket-Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfig.cmake
    INSTALL_DESTINATION cmake'
    PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfigVersion.cmake
    VERSION ${SIMPLE_SOCKET_VERSION}
    COMPATIBILITY SameMajorVersion)

install(TARGETS Simple-Socket
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib )
install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfigVersion.cmake
        DESTINATION cmake )

export(PACKAGE Simple-Socket)

if(SIMPLE_SOCKET_EXAMPLES)
    add_subdirectory(examples)
endif()

if(SIMPLE_SOCKET_TEST OR SIMPLE_SOCKET_COVERAGE)
    add_subdirectory(tests)
endif()
