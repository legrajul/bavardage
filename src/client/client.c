#include "lib_client.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include <pthread.h>

SocketTCP *client_sock;
pthread_t threadSend, threadRecv;

int connect (const char* addr, const int port) {
	if ((client_sock = creerSocketTCP() == NULL) {
        perror("creerSocketTCP");
        return -1;
    }

    if (connectSocketTCP(client_sock, addr, port) == -1 ) {
    	perror("connectSocketTCP");
        return -1;
    }

    return 0;
}

int send_message (const char* mess) {
	//rien
}