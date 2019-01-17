# Setup source files
set(TESTER ${PROJECT_NAME}-Tester)
set(COVERAGE ${PROJECT_NAME}-Coverage)
set(TESTER_SOURCES "main.cpp" "unicast.cpp" "multicast.cpp")

# Tester
if(SIMPLE_SOCKET_TEST)
    enable_testing()
    add_executable(${TESTER} ${TESTER_SOURCES})
    target_include_directories(${TESTER} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_options(${TESTER} PRIVATE ${WARNING_FLAGS})
    target_link_libraries(${TESTER} Simple-Socket ${THREAD_LIB})
    add_test(NAME test COMMAND ${TESTER})
endif()

# Coverage
if( SIMPLE_SOCKET_COVERAGE
AND CMAKE_COMPILER_IS_GNUCXX
AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "7"
AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "8" )
    include(CodeCoverage)
    set(COVERAGE_LCOV_EXCLUDES ${TESTER_SOURCES} "*7/*")

    add_executable(${COVERAGE} ${SIMPLE_SOCKET_SOURCES} ${TESTER_SOURCES})
    target_compile_options(${COVERAGE} PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage)
    target_link_libraries(${COVERAGE} ${THREAD_LIB} gcov)

    SETUP_TARGET_FOR_COVERAGE_LCOV(NAME coverage EXECUTABLE ${COVERAGE})
endif()
