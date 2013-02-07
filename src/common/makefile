FLAGS = -Wall -std=c99

all: SocketTCP.o user_manager.o room_manager.o test_rooms libsockettcp.o

libsockettcp.o: SocketTCP.c SocketTCP.h
	gcc -c -fPIC SocketTCP.c -o libsockettcp.o

test_rooms.o: test_rooms.c
	gcc $< -c $(FLAGS)

test_rooms: user_manager.o room_manager.o test_rooms.o 
	gcc $^ -o $@ $(FLAGS)

%.o: %.c
	gcc -c $< $(FLAGS)

clean:
	rm -rf *.o *~
