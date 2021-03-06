# Partie libclient.a
set(libclient_SRCS ../common/SocketTCP.c libclient.c ../common/SocketTCP.h libclient.h)
add_library(client STATIC ${libclient_SRCS})

# Partie libclientsec.a
find_package(OpenSSL)
set(libclientsec_SRCS ../common/user_manager.c ../common/room_manager.c ../common/room_manager_sec.c ../common/SocketTCP.c libclient.c ../common/commonsec.c libclientsec.c ../common/SocketTCP.h libclient.h)
add_library(clientsec STATIC ${libclientsec_SRCS})


# Partie client sécurisé (TEST)
set(CLIENTSEC_CLI_FLAGS -w -pthread -std=gnu99)
add_definitions(${CLIENTSEC_CLI_FLAGS})
set(client_cli_sec_SRCS ../common/user_manager.c ../common/room_manager.c ../common/room_manager_sec.c   clientsec-cli.c)
find_package(Threads)
add_executable(clientsec-cli ${client_cli_sec_SRCS})
target_link_libraries(clientsec-cli ${CMAKE_THREAD_LIBS_INIT} client clientsec ssl crypto)

# Partie client lignes de commandes
set(CLIENT_CLI_FLAGS -w -pthread -std=c99)
add_definitions(${CLIENT_CLI_FLAGS})
set(client_cli_SRCS client-cli.c)
find_package(Threads)
add_executable(client-cli ${client_cli_SRCS})
target_link_libraries(client-cli ${CMAKE_THREAD_LIBS_INIT} client)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# rien
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# Partie Client graphique
find_package(Vala REQUIRED)
include(ValaVersion)
ensure_vala_version("0.18.0" MINIMUM)
include(ValaPrecompile)
vala_precompile(VALA_C 
  Threads/ReceiveThread.vala
  Threads/ReceiveSecThread.vala
  Widgets/ConnectionDialog.vala
  Widgets/NewRoomDialog.vala
  Widgets/JoinRoomDialog.vala
  Widgets/AboutDialog.vala
  Widgets/SendInvitationDialog.vala
  Client.vala
PACKAGES
  gtk+-3.0
  gee-1.0
  glib-2.0
  posix
OPTIONS
  --thread
  --target-glib=2.32
CUSTOM_VAPIS
  libclient.vapi
  libclientsec.vapi
)

find_package(PkgConfig)
pkg_check_modules(DEPS REQUIRED gtk+-3.0 gee-1.0 glib-2.0 posix)
set(CFLAGS ${DEPS_CFLAGS} ${DEPS_CFLAGS_OTHER} -w)

set(LIB_PATHS ${DEPS_LIBRARY_DIRS} ${CMAKE_BIN_DIR})
link_directories(${LIB_PATHS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
remove_definitions(${CLIENT_CLI_FLAGS})
add_definitions(${CFLAGS})

add_executable(Client ${VALA_C})
target_link_libraries(Client ${DEPS_LIBRARIES} client clientsec gthread-2.0 ssl crypto)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/interface.ui DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS Client RUNTIME DESTINATION bin)

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
