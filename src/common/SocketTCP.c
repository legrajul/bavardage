/**
 * L3 Info Université de Rouen
 * TP 2 Réseaux
 * @author Julien Legras
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <string.h>

#include "SocketTCP.h"

SocketTCP *allocSocketTCP() {
	SocketTCP *s;
	id adr = { NULL, NULL, -1 };
	s = (SocketTCP *) malloc(sizeof(SocketTCP));
	s->socket = -1;
	s->local = s->distant = adr;
	return s;
}

SocketTCP *creerSocketTCP() {
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return NULL;
	}
	SocketTCP *s;
	s = allocSocketTCP();
	s->socket = sock;
	return s;
}

char *getLocalName(SocketTCP *socket) {
	return socket->local.name;
}

char *getLocalIP(SocketTCP *socket) {
	return socket->local.ip;
}

int getLocalPort(SocketTCP *socket) {
	return socket->local.port;
}

char *getDistantName(SocketTCP *socket) {
	return socket->distant.name;
}

char *getDistantIP(SocketTCP *socket) {
	return socket->distant.ip;
}

int getDistantPort(SocketTCP *socket) {
	return socket->distant.port;
}

int isConnected(SocketTCP *socket) {
	return (socket->distant.port == -1) ? 0 : 1;
}

int initSockAddr(const char *adresse, int port, struct sockaddr_in *in) {
	in->sin_family = AF_INET;
	memset(in->sin_zero, 8, 1);
	in->sin_port = htons(port);
	
	struct hostent *h;
	if ((h = gethostbyname(adresse)) == NULL) {
		// c'est une erreur
        return 0;
	} else {
       	if ((in->sin_addr.s_addr = inet_addr(adresse)) == -1) {
			// nom de domaine
            in->sin_addr = *((struct in_addr *) h->h_addr_list[0]);
		} else {
            // adresse pointée
        }
	}
	return 1;
}

id getIdBySockAddr(const struct sockaddr_in *in) {
	id i;
	struct hostent *h;
	if ((h = gethostbyaddr((void *) &in, sizeof(in), AF_INET)) == NULL) {
		i.name = NULL;
		i.ip = NULL;
		i.port = -1;
	} else {
		i.name = (char *) malloc(strlen(h->h_name));
		strcpy(i.name, h->h_name);
		i.ip = inet_ntoa(in->sin_addr);
		i.port = ntohs(in->sin_port);
	}
	return i;
}

id getIdByDescriptor(int socket) {
	id i;
	struct sockaddr_in in;
	socklen_t len = sizeof(in);
	if (getsockname(socket, (struct sockaddr *) &in, &len) == -1) {
		i.name = NULL;
		i.ip = NULL;
		i.port = -1;
	} else {
		i = getIdBySockAddr(&in);
	}
	return i;
}

int connectSocketTCP(SocketTCP *socket, const char *addresse, int port) {
	struct sockaddr_in in;
	if (initSockAddr(addresse, port, &in) == 0) {
		return -1;
	}
	
	/*struct hostent *h;
	if ((h = gethostbyname(addresse)) == NULL) {
		printf("erreur hostent\n");
	}*/

	if (connect(socket->socket, (struct sockaddr *) &in, sizeof(in)) == -1) {
		return -1;
	}

	socket->distant = getIdBySockAddr(&in);
	socket->local = getIdByDescriptor(socket->socket);
	/*if (initSockAddr(socket->local.ip, socket->local.port, &in) == 0) {
		return -1;
	}
	if (bind(socket->socket, (struct sockaddr *) &in, sizeof(in)) == -1) {
		return -1;
	}*/

	return 0;
}

SocketTCP *creerSocketEcouteTCP(const char *addresse, int port) {
	SocketTCP *s;
	s = creerSocketTCP();
	struct sockaddr_in in;
    if (initSockAddr(addresse, port, &in) == 0) {
        return NULL;
    }

    if (bind(s->socket, (struct sockaddr *) &in, sizeof(in)) == -1) {
        return NULL;
    }

	if (listen(s->socket, SIZE_QUEUE) == -1) {
		perror ("listen");
		return NULL;
	}

	return s;
}

SocketTCP *acceptSocketTCP(SocketTCP *socket) {
	SocketTCP *s;
	struct sockaddr_in in2;
	socklen_t in2_len = sizeof(in2);
    int co, sock;
	
    sock = socket->socket;
    s = creerSocketTCP();
    if ((co = accept(sock, (struct sockaddr *) &in2, &in2_len)) == -1) {
        return NULL;
	}
	
    s->socket = co;
    // les deux instructions suivantes prennent plus ou moins du temps à s'exécuter...
	s->local = getIdByDescriptor(co);
	s->distant = getIdBySockAddr(&in2);
	return s;
}

int writeSocketTCP(SocketTCP *socket, const char *buffer, int length) {
	return write(socket->socket, buffer, length);
}

int readSocketTCP(SocketTCP *socket, char *buffer, int length) {
	return recv(socket->socket, buffer, length, 0);
}

int closeSocketTCP(SocketTCP *socket) {
	int ret;
    if ((ret = shutdown(socket->socket, SHUT_RDWR)) == -1) {
        perror("shutdown");
        return -1;
    }
	free(socket);
    return ret;
}

