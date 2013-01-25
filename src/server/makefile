CC=gcc
OPTS= -Wall -std=c99 -pthread -l sqlite3

all: server

server: ../common/SocketTCP.o ../common_server/mysqlite.o server.o
	$(CC) $^ -o $@ $(OPTS)

server.o: server.c lib_server.h
	$(CC) $< -c

clean:
	rm server server.o
