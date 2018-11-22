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
    add_compile_options(-Wall -Wextra)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_EXECUTABLE_SUFFIX .run)
    set(THREAD_LIB pthread)

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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()

    if(APPLE)
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)
    set(PROJECT_LIBS Ws2_32.lib)

    if(MSVC)
        add_compile_options(/std:c++17 -W4)
    else()
        message( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

# Setup versioning.
set(BUILD_MAJOR "2")
set(BUILD_MINOR "0")
set(BUILD_VERSION "0")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

# Setup source files
include_directories(src)

set(DATA_FILE docs/README.md )
FILE(GLOB SIMPLE_SOCKET_HEADERS "src/*.h")
FILE(GLOB SIMPLE_SOCKET_SOURCES "src/*.cpp")
LIST(APPEND SIMPLE_SOCKET_SOURCES ${SIMPLE_SOCKET_HEADERS}) # append to sources so that dependency checks work on headers

FILE(GLOB DNS_SOURCE "examples/dns/*")
FILE(GLOB HTTP_SOURCE "examples/http/*")

# Project Options
OPTION(BUILD_SHARED_LIBS "Build Simple-Socket as a shared library." OFF)
OPTION(SIMPLE_SOCKET_EXAMPLES "Build the examples" ON)
OPTION(SIMPLE_SOCKET_TEST "Build the tester" OFF)

# Build the lib and install into configured prefix
ADD_LIBRARY(Simple-Socket ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
install(TARGETS Simple-Socket
                RUNTIME DESTINATION bin
                ARCHIVE DESTINATION lib )
install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
TARGET_LINK_LIBRARIES(Simple-Socket ${PROJECT_LIBS})
set_target_properties(Simple-Socket PROPERTIES VERSION ${BUILD_VERSION} SOVERSION ${BUILD_MAJOR})

# Examples
if(SIMPLE_SOCKET_EXAMPLES)
    ADD_EXECUTABLE(asyncclient-example examples/RecvAsync.cpp)
    TARGET_LINK_LIBRARIES(asyncclient-example Simple-Socket ${THREAD_LIB})

    ADD_EXECUTABLE(querydaytime-example examples/QueryDayTime.cpp)
    TARGET_LINK_LIBRARIES(querydaytime-example Simple-Socket)

    ADD_EXECUTABLE(echoserver-example examples/EchoServer.cpp)
    TARGET_LINK_LIBRARIES(echoserver-example Simple-Socket ${THREAD_LIB})

    ADD_EXECUTABLE(echoclient-example examples/EchoClient.cpp)
    TARGET_LINK_LIBRARIES(echoclient-example Simple-Socket)

    ADD_EXECUTABLE(multicast-example examples/Multicast.cpp)
    TARGET_LINK_LIBRARIES(multicast-example Simple-Socket ${THREAD_LIB})

    ADD_EXECUTABLE(dnsclient-example ${DNS_SOURCE})
    TARGET_LINK_LIBRARIES(dnsclient-example Simple-Socket)

    ADD_EXECUTABLE(httpclient-example examples/HttpClient.cpp ${HTTP_SOURCE})
    target_include_directories(httpclient-example PRIVATE examples/http/)
    TARGET_LINK_LIBRARIES(httpclient-example Simple-Socket)
endif()

# Tester
if(SIMPLE_SOCKET_TEST)
    ENABLE_TESTING()
    ADD_EXECUTABLE(Simple-Socket-tester tests/main.cpp ${SIMPLE_SOCKET_SOURCES})

    if(CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "7"
    AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "8" )
        message("generating coverage")
        include(CodeCoverage)
        set(GCOV_LIB gcov)
        APPEND_COVERAGE_COMPILER_FLAGS()
        set(COVERAGE_LCOV_EXCLUDES '*tests/*' '*examples/*' '*7/*')
        SETUP_TARGET_FOR_COVERAGE_LCOV(NAME ${PROJECT_NAME}-Coverage EXECUTABLE Simple-Socket-tester)
    endif()

    TARGET_LINK_LIBRARIES(Simple-Socket-tester ${THREAD_LIB} ${GCOV_LIB})
    ADD_TEST(NAME Socket-Tester COMMAND Simple-Socket-tester)

endif()
