FLAGS = -Wall -std=c99

all: SocketTCP.o user_manager.o room_manager.o

%.o: %.c
	gcc -c $< $(FLAGS)

clean:
	$(RM) *.o *~
