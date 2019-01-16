# Setup source files
set(TESTER_SOURCES "main.cpp" "unicast.cpp" "multicast.cpp")
set(SIMPLE_SOCKET_TESTER_SOURCES ${TESTER_SOURCES} PARENT_SCOPE)

# Tester
if(SIMPLE_SOCKET_TEST)
enable_testing()
    add_executable(Simple-Socket-Tester ${TESTER_SOURCES})
    target_include_directories(Simple-Socket-Tester PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_options(Simple-Socket-Tester PRIVATE ${WARNING_FLAGS})
    target_link_libraries(Simple-Socket-Tester Simple-Socket ${THREAD_LIB})
    add_test(NAME Socket-Tester COMMAND Simple-Socket-Tester)
endif()
