cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Simple-Socket)

# CMake Settings
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif()

#set(CMAKE_VERBOSE_MAKEFILE ON)

# OS and compiler checks.
if(UNIX)
    # linux / normal unix
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_STANDARD 17)
    IF(CMAKE_BUILD_TYPE MATCHES Debug)
      message("Building 'Debug' configuration.")
      add_compile_options(-g -D_DEBUG)
    ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
      message("Building 'Release' configuration.")
      add_compile_options(-O3)
    ELSE()
      set(CMAKE_BUILD_TYPE Release)
      message("Defaulting to 'Release' configuration.")
    ENDIF()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_definitions(-D_CLANG)
    endif()

    add_compile_options(-Wall -Wextra)

    if(CYGWIN)
        # Special Cygwin stuff here
    elseif(APPLE)
        # Special Apple stuff here
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)
    SET(PROJECT_LIBS Ws2_32.lib)
    if(MINGW)
        # Special MINGW stuff here
    elseif(MSVC)
        # Special MSVC stuff here
        add_compile_options(/std:c++17 -W4)
    else()
        # No idea what it is, but there's a good chance it's too weird.
        MESSAGE( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
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
SET_SOURCE_FILES_PROPERTIES( ${SIMPLE_SOCKET_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE ) # mark headers as headers...
LIST(APPEND SIMPLE_SOCKET_SOURCES ${SIMPLE_SOCKET_HEADERS}) # append to sources so that dependency checks work on headers

FILE(GLOB DNS_SOURCE "examples/dns/*")
FILE(GLOB HTTP_SOURCE "examples/http/*")

# Project Options
OPTION(SIMPLE_SOCKET_SHARED "Build Simple-Socket lib as shared." OFF)
OPTION(SIMPLE_SOCKET_DEP_ONLY "Build for use inside other CMake projects as a dependency." OFF)
OPTION(SIMPLE_SOCKET_EXAMPLES "Build the examples" ON)
OPTION(SIMPLE_SOCKET_TEST "Build the tester" OFF)

# Build the lib and install into configured prefix
if(SIMPLE_SOCKET_SHARED)
    if(SIMPLE_SOCKET_DEP_ONLY)
        ADD_LIBRARY(Simple-Socket SHARED ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
        install(TARGETS Simple-Socket
                        RUNTIME DESTINATION bin
                        LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib/${BUILD_VERSION} )
        install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
    else()
        ADD_LIBRARY(Simple-Socket SHARED ${SIMPLE_SOCKET_SOURCES})
    endif()
else(NOT SIMPLE_SOCKET_SHARED)
    if(SIMPLE_SOCKET_DEP_ONLY)
        ADD_LIBRARY(Simple-Socket STATIC ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
        install(TARGETS Simple-Socket
                        LIBRARY DESTINATION lib
                        ARCHIVE DESTINATION lib/${BUILD_VERSION} )
        install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
    else()
        ADD_LIBRARY(Simple-Socket STATIC ${SIMPLE_SOCKET_SOURCES})
    endif()
endif()
TARGET_LINK_LIBRARIES(Simple-Socket ${PROJECT_LIBS})
set_target_properties(Simple-Socket PROPERTIES VERSION ${BUILD_VERSION} SOVERSION ${BUILD_MAJOR})

# Examples
if(NOT SIMPLE_SOCKET_DEP_ONLY AND SIMPLE_SOCKET_EXAMPLES)
   if(UNIX)
        ADD_EXECUTABLE(asyncclient-example.run examples/RecvAsync.cpp)
        TARGET_LINK_LIBRARIES(asyncclient-example.run Simple-Socket pthread)

        ADD_EXECUTABLE(querydaytime-example.run examples/QueryDayTime.cpp)
        TARGET_LINK_LIBRARIES(querydaytime-example.run Simple-Socket)

        ADD_EXECUTABLE(echoserver-example.run examples/EchoServer.cpp)
        TARGET_LINK_LIBRARIES(echoserver-example.run Simple-Socket pthread)

        ADD_EXECUTABLE(echoclient-example.run examples/EchoClient.cpp)
        TARGET_LINK_LIBRARIES(echoclient-example.run Simple-Socket)

        ADD_EXECUTABLE(multicast-example.run examples/Multicast.cpp)
        TARGET_LINK_LIBRARIES(multicast-example.run Simple-Socket pthread)

        ADD_EXECUTABLE(dnsclient-example.run ${DNS_SOURCE})
        TARGET_LINK_LIBRARIES(dnsclient-example.run Simple-Socket)

        ADD_EXECUTABLE(httpclient-example.run examples/HttpClient.cpp ${HTTP_SOURCE})
        target_include_directories(httpclient-example.run PRIVATE examples/http/)
        TARGET_LINK_LIBRARIES(httpclient-example.run Simple-Socket)
    elseif(WIN32)
        ADD_EXECUTABLE(asyncclient-example examples/RecvAsync.cpp)
        TARGET_LINK_LIBRARIES(asyncclient-example Simple-Socket)

        ADD_EXECUTABLE(querydaytime-example examples/QueryDayTime.cpp)
        TARGET_LINK_LIBRARIES(querydaytime-example Simple-Socket)

        ADD_EXECUTABLE(echoserver-example examples/EchoServer.cpp)
        TARGET_LINK_LIBRARIES(echoserver-example Simple-Socket)

        ADD_EXECUTABLE(echoclient-example examples/EchoClient.cpp)
        TARGET_LINK_LIBRARIES(echoclient-example Simple-Socket)

        ADD_EXECUTABLE(multicast-example examples/Multicast.cpp)
        TARGET_LINK_LIBRARIES(multicast-example Simple-Socket)

        ADD_EXECUTABLE(dnsclient-example ${DNS_SOURCE})
        TARGET_LINK_LIBRARIES(dnsclient-example Simple-Socket)

        ADD_EXECUTABLE(httpclient-example examples/HttpClient.cpp ${HTTP_SOURCE})
        target_include_directories(httpclient-example PRIVATE examples/http/)
        TARGET_LINK_LIBRARIES(httpclient-example Simple-Socket)
    endif()
endif()

# Tester
if(NOT SIMPLE_SOCKET_DEP_ONLY AND SIMPLE_SOCKET_TEST)
    if(UNIX)
        ADD_EXECUTABLE(Simple-Socket-tester.run tests/main.cpp)
        TARGET_LINK_LIBRARIES(Simple-Socket-tester.run Simple-Socket pthread)
    elseif(WIN32)
        ADD_EXECUTABLE(Simple-Socket-tester tests/main.cpp)
        TARGET_LINK_LIBRARIES(Simple-Socket-tester Simple-Socket)
    endif()
endif()
