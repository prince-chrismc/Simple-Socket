# Setup source files
set(TESTER ${PROJECT_NAME}-Tester)
set(COVERAGE ${PROJECT_NAME}-Coverage)
set(TESTER_SOURCES
    "main.cpp"
    "unicast.cpp"
    "multicast.cpp"
    "async.cpp")

set(SIMPLE_SOCKET_STRING_VIEW
    ON
    CACHE BOOL "Required by tester and coverage" FORCE)

include(FetchContent)
FetchContent_Declare(Catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.9.1
)

FetchContent_MakeAvailable(Catch)

include(CTest)
include(Catch)

# Tester
if(SIMPLE_SOCKET_TEST)
    add_executable(${TESTER} ${TESTER_SOURCES})
    target_compile_features(${TESTER} PRIVATE cxx_std_17 )
    target_include_directories(${TESTER} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_options(${TESTER} PRIVATE ${WARNING_FLAGS})
    target_link_libraries(${TESTER} Simple-Socket ${THREAD_LIB} Catch2::Catch2)
    catch_discover_tests(${TESTER})
endif()

# Coverage
if(SIMPLE_SOCKET_COVERAGE
   AND CMAKE_COMPILER_IS_GNUCXX
   AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER "7"
   AND ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "8")
  include(CodeCoverage)
  set(COVERAGE_LCOV_EXCLUDES ${TESTER_SOURCES} "*catch2/*" "*7/*")

    add_executable(${COVERAGE} ${SIMPLE_SOCKET_SOURCES} ${TESTER_SOURCES})
    target_include_directories(${COVERAGE} PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_features(${COVERAGE} PRIVATE cxx_std_17 )
    target_compile_options(${COVERAGE} PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage -DSTRING_VIEW)
    target_link_libraries(${COVERAGE} ${THREAD_LIB} gcov Catch2::Catch2)

  setup_target_for_coverage_lcov(NAME
                                 coverage
                                 EXECUTABLE
                                 ${COVERAGE})
endif()
