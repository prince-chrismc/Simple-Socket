# Enable clang-tidy
find_program(CLANG_TIDY_EXE
             NAMES "clang-tidy" "clang-tidy-8" "clang-tidy-7"
             DOC "Path to clang-tidy executable")

if(CLANG_TIDY_EXE)
  message(AUTHOR_WARNING "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_CURRENT_BINARY_DIR};")
  # set(CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY} CACHE STRING "" FORCE)
else()
  message(AUTHOR_WARNING "clang-tidy not found!")
  set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
endif()

# Enable cppcheck
find_program(CPPCHECK_EXE NAMES "cppcheck" DOC "Path to cppcheck executable")

if(CPPCHECK_EXE)
  message(AUTHOR_WARNING "cppcheck found: ${CPPCHECK_EXE}")
  add_custom_target(
    cppcheck
    COMMAND "${CPPCHECK_EXE}" "-j4" "--enable=all" "--inconclusive"
            "--suppress=*:*catch.hpp"
            "--suppress=compareBoolExpressionWithInt:*tests/*.cpp"
            "--project=compile_commands.json" "--xml" "--output-file=report.xml"
    COMMAND "cppcheck-htmlreport" "--file=report.xml" "--report-dir=report"
            "--source-dir=..")
else()
  message(AUTHOR_WARNING "cppcheck not found!")
  # set(CXX_CPPCHECK "" CACHE STRING "" FORCE)
endif()

# set(CMAKE_CXX_CPPCHECK
# "cppcheck;--enable=all;--project=compile_commands.json")
# set(CMAKE_CXX_CPPCHECK "") set(CXX_CPPCHECK ${CMAKE_CXX_CPPCHECK} CACHE STRING
# "" FORCE)
