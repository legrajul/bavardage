FLAGS = -Wall -std=c99

all: SocketTCP.o

%.o: %.c
	gcc -c $< $(FLAGS)

clean:
	$(RM) *.o *~
