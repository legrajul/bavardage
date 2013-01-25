CC=gcc
OPTS= -Wall -std=c99 -pthread -l sqlite3

all: mysqlite.o

mysqlite.o: mysqlite.c mysqlite.h
	$(CC) $< -c $(OPTS)

clean:
	rm mysqlite.o

