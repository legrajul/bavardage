#~ VALAC=valac
#~ PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0
CC=gcc
OPTS= -Wall -pthread -std=gnu99

all: client

#~ Client: Client.vala interface.ui
	#~ $(VALAC) $(PKGS) Client.vala -o $@ -X -w

client: ../common/SocketTCP.o client.o 
	$(CC) $(OPTS) $^ -pthread -o $@

client.o: client.c lib_client.h ../common/common.h
	$(CC) -c $< $(OPTS)

#test_server: ../common/SocketTCP.o test_server.o
#	$(CC) $(OPTS) $^ -o $@

#test_server.o: test_server.c
#	$(CC) -c $< $(OPTS)

clean:
	rm Client client
