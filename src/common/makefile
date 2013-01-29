FLAGS = -Wall -std=c99

all: SocketTCP.o user_manager.o room_manager.o test_rooms

test_rooms.o: test_rooms.c
	gcc $< -c $(FLAGS)

test_rooms: user_manager.o room_manager.o test_rooms.o 
	gcc $^ -o $@ $(FLAGS)

%.o: %.c
	gcc -c $< $(FLAGS)

clean:
	rm -rf *.o *~
