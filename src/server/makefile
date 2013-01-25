CC=gcc
OPTS= -Wall -std=c99 -pthread

all: server

server: ../common/SocketTCP.o server.o
	$(CC) $(OPTS) $^ -o $@

server.o: server.c lib_server.h

clean:
	server
