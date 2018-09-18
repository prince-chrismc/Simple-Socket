cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(simple-socket)

# set up versioning.
set(BUILD_MAJOR "2")
set(BUILD_MINOR "0")
set(BUILD_VERSION "0")
set(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_VERSION})

set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(src)

SET(SIMPLE_SOCKET_HEADERS
src/ActiveSocket.h
src/Host.h
src/PassiveSocket.h
src/SimpleSocket.h
src/StatTimer.h
)

SET(SIMPLE_SOCKET_SOURCES
src/SimpleSocket.cpp
src/ActiveSocket.cpp
src/PassiveSocket.cpp
)

set(DATA_FILE
README.md
)

# mark headers as headers...
SET_SOURCE_FILES_PROPERTIES( ${SIMPLE_SOCKET_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE )
# append to sources so that dependency checks work on headers
LIST(APPEND SIMPLE_SOCKET_SOURCES ${SIMPLE_SOCKET_HEADERS})

# OS and compiler checks.
if(UNIX)
    # linux / normal unix
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_STANDARD 17)
    ADD_CUSTOM_TARGET(debug
      COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
      COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all)
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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    else()
        # No idea what it is, but there's a good chance it's too weird.
        MESSAGE( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

OPTION(SIMPLE_SOCKET_SHARED "Build simple-socket lib as shared." OFF)
OPTION(SIMPLE_SOCKET_DEP_ONLY "Build for use inside other CMake projects as dependency." OFF)

# make the lib
if(SIMPLE_SOCKET_SHARED)
    if(SIMPLE_SOCKET_DEP_ONLY)
        ADD_LIBRARY(simple-socket SHARED EXCLUDE_FROM_ALL ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
    else()
        ADD_LIBRARY(simple-socket SHARED ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
    endif()
else()
    if(SIMPLE_SOCKET_DEP_ONLY)
        ADD_LIBRARY(simple-socket STATIC EXCLUDE_FROM_ALL ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
    else()
        ADD_LIBRARY(simple-socket STATIC ${SIMPLE_SOCKET_SOURCES} ${DATA_FILE})
    endif()
endif()
TARGET_LINK_LIBRARIES(simple-socket ${PROJECT_LIBS})

# install into configured prefix
if(NOT SIMPLE_SOCKET_DEP_ONLY)
    install(TARGETS simple-socket ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
    install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
endif()

set_target_properties(simple-socket PROPERTIES VERSION ${BUILD_VERSION} SOVERSION ${BUILD_MAJOR})

OPTION(SIMPLE_SOCKET_EXAMPLES "Build the examples" ON)
if(NOT SIMPLE_SOCKET_DEP_ONLY)
    if(SIMPLE_SOCKET_EXAMPLES)
       if(UNIX)
            ADD_EXECUTABLE(asyncclient-example.run examples/RecvAsync.cpp)
            TARGET_LINK_LIBRARIES(asyncclient-example.run simple-socket pthread)

            ADD_EXECUTABLE(querydaytime-example.run examples/QueryDayTime.cpp)
            TARGET_LINK_LIBRARIES(querydaytime-example.run simple-socket)

            ADD_EXECUTABLE(echoserver-example.run examples/EchoServer.cpp)
            TARGET_LINK_LIBRARIES(echoserver-example.run simple-socket pthread)

            ADD_EXECUTABLE(echoclient-example.run examples/EchoClient.cpp)
            TARGET_LINK_LIBRARIES(echoclient-example.run simple-socket)

            ADD_EXECUTABLE(multicast-example.run examples/Multicast.cpp)
            TARGET_LINK_LIBRARIES(multicast-example.run simple-socket pthread)

            FILE(GLOB DNS "examples/dns/*")
            ADD_EXECUTABLE(dnsclient-example.run ${DNS})
            TARGET_LINK_LIBRARIES(dnsclient-example.run simple-socket)

            FILE(GLOB HTTP "examples/http/*")
            ADD_EXECUTABLE(httpclient-example.run examples/HttpClient.cpp ${HTTP})
            TARGET_LINK_LIBRARIES(httpclient-example.run simple-socket)

            ADD_EXECUTABLE(simple-socket-tester.run tests/main.cpp)
            TARGET_LINK_LIBRARIES(simple-socket-tester.run simple-socket pthread)
        elseif(WIN32)
            ADD_EXECUTABLE(asyncclient-example examples/RecvAsync.cpp)
            TARGET_LINK_LIBRARIES(asyncclient-example simple-socket)

            ADD_EXECUTABLE(querydaytime-example examples/QueryDayTime.cpp)
            TARGET_LINK_LIBRARIES(querydaytime-example simple-socket)

            ADD_EXECUTABLE(echoserver-example examples/EchoServer.cpp)
            TARGET_LINK_LIBRARIES(echoserver-example simple-socket)

            ADD_EXECUTABLE(echoclient-example examples/EchoClient.cpp)
            TARGET_LINK_LIBRARIES(echoclient-example simple-socket)

            ADD_EXECUTABLE(multicast-example examples/Multicast.cpp)
            TARGET_LINK_LIBRARIES(multicast-example simple-socket)

            FILE(GLOB DNS "examples/dns/*")
            ADD_EXECUTABLE(dnsclient-example ${DNS})
            TARGET_LINK_LIBRARIES(dnsclient-example simple-socket)

            FILE(GLOB HTTP "examples/http/*")
            ADD_EXECUTABLE(httpclient-example examples/HttpClient.cpp ${HTTP})
            TARGET_LINK_LIBRARIES(httpclient-example simple-socket)

            ADD_EXECUTABLE(simple-socket-tester tests/main.cpp)
            TARGET_LINK_LIBRARIES(simple-socket-tester simple-socket)
        endif()
    endif()
endif()
