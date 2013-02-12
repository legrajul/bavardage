VALAC=valac
PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0 --thread
CC=gcc
OPTS= -Wall -pthread -std=gnu99

all: client-cli Client

Client: Client.vala interface.ui libclient.a
	$(VALAC)  -X -w -X -I. -X -L. -X -lclient $(PKGS) -o $@ Client.vala libclient.vapi --basedir ./ --target-glib=2.32

libclient.a: libclient.o ../common/libsockettcp.o
	ar rcs libclient.a ../common/libsockettcp.o libclient.o

libclient.o: libclient.c libclient.h
	gcc -c -fPIC libclient.c -o libclient.o

client-cli: ../common/SocketTCP.o libclient.o client-cli.o
	$(CC) $(OPTS) $^ -pthread -o $@

client-cli.o: client-cli.c libclient.h
	$(CC) -c $< $(OPTS)

clean:
	rm Client client-cli libclient.so
