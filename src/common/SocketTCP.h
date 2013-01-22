
#ifndef SOCKET_TCP
#define SOCKET_TCP

/*
 *  SocketTCP.h
 *
 *  Created by Said Abdeddaim on 19/03/2011.
 *  Copyright 2011 Said Abdeddaim. All rights reserved.
 *
 */
 
#define ADRESSE_IP	1
#define ADRESSE_NOM	0
#define ADRESSE_NON_VALIDE -1

#define SIZE_QUEUE 100

typedef struct {
	char *name;
	char *ip;
	int port;
} id;

typedef struct {
	int socket;
	id local;
	id distant;
} SocketTCP;

SocketTCP *creerSocketTCP();

char *getLocalName(SocketTCP *socket);

char *getLocalIP(SocketTCP *socket);

int getLocalPort(SocketTCP *socket);

char *getDistantName(SocketTCP *socket);

char *getDistantIP(SocketTCP *socket);

int getDistantPort(SocketTCP *socket);

int isConnected(SocketTCP *socket);

int connectSocketTCP(SocketTCP *socket, const char *addresse, int port);

SocketTCP *creerSocketEcouteTCP(const char *addresse, int port);

SocketTCP *acceptSocketTCP(SocketTCP *socket);

int writeSocketTCP(SocketTCP *socket, const char *buffer, int length);

int readSocketTCP(SocketTCP *socket, char *buffer, int length);

int closeSocketTCP(SocketTCP *socket);

#endif
