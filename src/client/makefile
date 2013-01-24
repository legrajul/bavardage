VALAC=valac
PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0
CC=gcc
OPTS= -Wall -std=c99 -pthread

all: Client client server

Client: Client.vala interface.ui
	$(VALAC) $(PKGS) Client.vala -o $@

client: ../common/SocketTCP.o client.o 
	$(CC) $(OPTS) $^ -pthread -o $@

client.o: client.c lib_client.h
	$(CC) -c $< $(OPTS)

server: ../common/SocketTCP.o server.o
	$(CC) $(OPTS) $^ -pthread -o $@

server.o: ../server/server.c ../server/lib_server.h
	$(CC) -c $< $(OPTS)

clean:
	rm Client client server
