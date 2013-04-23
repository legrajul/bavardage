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


IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Mac OS X specific code
SET(OperatingSystem "Mac OS X")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


add_subdirectory (server)
add_subdirectory (serversec)

add_subdirectory (client)

# Copie du certificat du serveur sécurisé
file(COPY ../certifs/bavardageserversec_certif.pem DESTINATION serversec/)
file(COPY ../certifs/bavardageserversec.pem DESTINATION serversec/)

# Copie du certificat racine dans les dossiers l'utilisant
file(COPY ../certifs/root.pem DESTINATION client/)
file(COPY ../certifs/root.pem DESTINATION serversec/)

# PARTIE DE TESTS : copie du certificat utilisateur
file(COPY ../certifs/toto_certif.pem DESTINATION client/)
file(COPY ../certifs/toto.pem DESTINATION client/)
