CC=gcc
OPTS= -Wall -std=c99 -pthread -l sqlite3

all: server

server: ../common/SocketTCP.o ../common_server/mysqlite.o server.o ../common/user_manager.o ../common/room_manager.o
	$(CC) $^ -o $@ $(OPTS)

server.o: server.c lib_server.h ../common/common.h ../common/user_manager.h ../common/room_manager.h
	$(CC) $< -c

clean:
	rm -rf server *.o
