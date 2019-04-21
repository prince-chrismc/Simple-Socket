include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include/)
set(LIBRARY_INSTALL_DIR lib/)
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/Simple-Socket-Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfig.cmake
  INSTALL_DESTINATION
  cmake'
  PATH_VARS
  INCLUDE_INSTALL_DIR
  LIBRARY_INSTALL_DIR)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfigVersion.cmake
  VERSION ${SIMPLE_SOCKET_VERSION}
  COMPATIBILITY SameMajorVersion)

install(TARGETS Simple-Socket RUNTIME DESTINATION bin ARCHIVE DESTINATION lib)
install(FILES ${SIMPLE_SOCKET_HEADERS} DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Simple-SocketConfigVersion.cmake
        DESTINATION cmake)

export(PACKAGE Simple-Socket)
