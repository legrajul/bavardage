VALAC=valac
PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0
CC=gcc
OPTS= -Wall -pthread -std=gnu99

all: client-cli

Client: Client.vala interface.ui
	$(VALAC) $(PKGS) Client.vala -o $@ -X -w

client-cli: ../common/SocketTCP.o lib_client.o client-cli.o
	$(CC) $(OPTS) $^ -pthread -o $@

client-cli.o: client-cli.c lib_client.h
	$(CC) -c $< $(OPTS)

lib_client.o: lib_client.c lib_client.h ../common/common.h
	$(CC) -c $< $(OPTS)

#test_server: ../common/SocketTCP.o test_server.o
#	$(CC) $(OPTS) $^ -o $@

#test_server.o: test_server.c
#	$(CC) -c $< $(OPTS)

clean:
	rm Client client-cli
