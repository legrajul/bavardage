FLAGS = -Wall -std=c99

all: SocketTCP.o room_manager.o

%.o: %.c
	gcc -c $< $(FLAGS)

clean:
	$(RM) *.o *~
