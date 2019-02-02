# Examples
set(SIMPLE_SOCKET_STRING_VIEW ON CACHE BOOL "Required by examples" FORCE)

add_executable(asyncclient-example RecvAsync.cpp)
target_compile_features(asyncclient-example PRIVATE cxx_std_17 )
target_include_directories(asyncclient-example PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(asyncclient-example Simple-Socket ${THREAD_LIB})

add_executable(querydaytime-example QueryDayTime.cpp)
target_compile_features(querydaytime-example PRIVATE cxx_std_17 )
target_include_directories(querydaytime-example PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(querydaytime-example Simple-Socket)

add_executable(echoserver-example EchoServer.cpp)
target_compile_features(echoserver-example PRIVATE cxx_std_17 )
target_include_directories(echoserver-example PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(echoserver-example Simple-Socket ${THREAD_LIB})

add_executable(echoclient-example EchoClient.cpp)
target_compile_features(echoclient-example PRIVATE cxx_std_17 )
target_include_directories(echoclient-example PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(echoclient-example Simple-Socket)

add_executable(multicast-example Multicast.cpp)
target_compile_features(multicast-example PRIVATE cxx_std_17 )
target_include_directories(multicast-example PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(multicast-example Simple-Socket ${THREAD_LIB})
