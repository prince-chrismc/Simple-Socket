# Enable clang-tidy
find_program(CLANG_TIDY_EXE
             NAMES "clang-tidy" "clang-tidy-8" "clang-tidy-7"
             DOC "Path to clang-tidy executable")

if(CLANG_TIDY_EXE)
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set(
  CMAKE_CXX_CLANG_TIDY
  "${CLANG_TIDY_EXE};-checks=modernize-*;-header-filter=${CMAKE_SOURCE_DIR};-p=${CMAKE_CURRENT_BINARY_DIR}"
  )
  # set(CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY} CACHE STRING "" FORCE)
else()
  message(AUTHOR_WARNING "clang-tidy not found!")
  set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # delete it
endif()

add_custom_target(clangformat
                  COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_CURRENT_BINARY_DIR}
                          -checks=* -header-filter=${CMAKE_SOURCE_DIR}
                          ${SIMPLE_SOCKET_SOURCES})
add_custom_target(cppcheck
                  COMMAND "cppcheck" "--enable=all"
                          "--project=compile_commands.json" "--xml"
                          "2> report.xml"
                  COMMAND "cppcheck-htmlreport" "--file=report.xml"
                          "--report-dir=report" "--source-dir=..")

# set(CMAKE_CXX_CPPCHECK "cppcheck;--enable=all;--project=compile_commands.json")
# set(CMAKE_CXX_CPPCHECK "")
# set(CXX_CPPCHECK ${CMAKE_CXX_CPPCHECK} CACHE STRING "" FORCE)
