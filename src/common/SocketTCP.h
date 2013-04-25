#ifndef SOCKET_TCP
#define SOCKET_TCP

/*
 *  SocketTCP.h
 *
 */

#define SIZE_QUEUE 100

typedef struct {
	int socket;
} SocketTCP;

SocketTCP *creerSocketTCP ();

int connectSocketTCP (SocketTCP *socket, const char *addresse, int port);

SocketTCP *creerSocketEcouteTCP (const char *addresse, int port);

SocketTCP *acceptSocketTCP (SocketTCP *socket);

int writeSocketTCP (SocketTCP *socket, const char *buffer, int length);

int readSocketTCP (SocketTCP *socket, char *buffer, int length);

int closeSocketTCP (SocketTCP *socket);

#endif
