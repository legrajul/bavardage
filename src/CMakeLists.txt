cmake_minimum_required (VERSION 2.6)
cmake_policy (VERSION 2.6)

project(bavardage)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (DATADIR "${CMAKE_INSTALL_PREFIX}/share")
set (PKGDATADIR "${DATADIR}/bavardage")
set (GETTEXT_PACKAGE "bavardage")
set (RELEASE_NAME "Alpha")
set (VERSION "0.1")
set (VERSION_INFO "Release")
set (CMAKE_C_FLAGS "-ggdb")
set (PREFIX ${CMAKE_INSTALL_PREFIX})
set (DOLLAR "$")

add_subdirectory (server)
add_subdirectory (client)