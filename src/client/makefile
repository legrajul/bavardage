VALAC=valac
PKGS= --pkg gtk+-3.0 --pkg gmodule-2.0 --pkg gee-1.0 --thread
CC=gcc
OPTS= -Wall -pthread -std=gnu99

all: client-cli Client

Client: Client.vala interface.ui libclient.so
	$(VALAC)  -X -w -X -I. -X -L. -X -lclient $(PKGS) -o $@ Client.vala libclient.vapi --basedir ./

libclient.so: libclient.o ../common/libsockettcp.o
	gcc -shared -o libclient.so ../common/libsockettcp.o libclient.o
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD

libclient.o: libclient.c libclient.h
	gcc -c -fPIC libclient.c -o libclient.o

libtest.so: test_vapi.o
	gcc -shared -o libtest.so test_vapi.o

test_vapi.o: test_vapi.c test_vapi.h
	gcc -c -fPIC test_vapi.c -o test_vapi.o

client-cli: ../common/SocketTCP.o libclient.o client-cli.o
	$(CC) $(OPTS) $^ -pthread -o $@

client-cli.o: client-cli.c lib_client.h
	$(CC) -c $< $(OPTS)

#test_server: ../common/SocketTCP.o test_server.o
#	$(CC) $(OPTS) $^ -o $@

#test_server.o: test_server.c
#	$(CC) -c $< $(OPTS)

clean:
	rm Client client-cli libclient.so
