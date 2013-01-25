CC=gcc
OPTS= -Wall -std=c99 -pthread

all: server

mysqlite: ../common/mysqlite.c ../common/mysqlite.h
	gcc $(CC_OPTS) -c ../common/mysqlite.c -o ../common/mysqlite -l sqlite3 -std=c99

server: ../common/SocketTCP.o server.o
	$(CC) $(OPTS) $^ -o $@ -l sqlite3

server.o: server.c lib_server.h



clean:
	server
