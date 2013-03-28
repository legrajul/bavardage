#include "secure_server.h"
#include "../common/SocketTCP.h"
#include "../common/commonsec.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include "openssl/ssl.h"
#include <openssl/ssl2.h>

SocketTCP *listen_socket;
pthread_mutex_t mutex;

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

int setup_ctx () {
    printf ("BEGIN setup_ctx\n");

    printf ("END setup_ctx\n");
}

void my_sigaction(int s) {
    switch (s) {
    case SIGINT:
        closeSocketTCP(listen_socket);
        exit(0);
        break;
    default:
        break;
    }
}

void *handle_connexion(void *param) {
    printf ("BEGIN handle_connexion\n");
    SocketTCP *s = (SocketTCP *) param;
    SSL *client_ssl;
    client_ssl = SSL_new(ctx);              /* get new SSL state with context */
    SSL_set_fd(client_ssl, s->socket);      /* set connection socket to SSL state */

    char buf[1024], reply[1024];
    int bytes = 0;
    if ( SSL_accept(client_ssl) <= 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {

	// TODO attention code de test
        bytes = SSL_read(client_ssl, buf, sizeof(buf)); /* get request */
	printf ("SSL_read : %d bytes\n", bytes);
        if ( bytes > 0 ) {
            buf[bytes] = 0;
            printf("Client msg: \"%s\"\n", buf);
	    sprintf (reply, "bonjour %s", buf);
            SSL_write(ssl, reply, strlen(reply)); /* send reply */
        } else {
            ERR_print_errors_fp(stderr);
	}
    }

    int sd = SSL_get_fd(client_ssl);       /* get socket connection */
    /* while (1) { */
        /* char *buf; */
        /* int ret = SSL_read (ssl, buf, sizeof (buf)); */
        /* if (ret > 0) { */
            /* printf ("Message reçu = %s\n", buf); */
            /* break; */
        /* } */
    /* } */

    printf ("END handle_connexion\n");
}

void new_thread(SocketTCP *socket) {
    int ret;

    pthread_attr_t attr;
    if ((ret = pthread_attr_init(&attr)) != 0) {
        fprintf(stderr, "pthread_attr_init: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // On dÃ©tache le thread afin de ne pas avoir Ã  faire de join
    if ((ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
        != 0) {
        fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    pthread_t t;
    if ((ret = pthread_create(&t, NULL, handle_connexion, (void*) socket))
        != 0) {
        fprintf(stderr, "pthead_create: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    if ((ret = pthread_attr_destroy(&attr)) != 0) {
        fprintf(stderr, "pthread_attr_destroy: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
}


int start_listening(const char *addr, int port) {
    SocketTCP *client;

    if ((listen_socket = creerSocketEcouteTCP(addr, port)) == NULL) {
        perror("creerSocketEcouteTCP");
        return -1;
    }

    ctx = initialize_ctx (CertFile, KeyFile, "bavardage");

    ssl = SSL_new(ctx);
    sbio = BIO_new_socket(listen_socket->socket, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    (void) signal(SIGINT, my_sigaction);
    pthread_mutex_init(&mutex, NULL);
    while (1) {
        client = acceptSocketTCP(listen_socket);
        printf("New connection\n");
        new_thread(client);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    setup_ctx ();
    if (argc < 3) {
        fprintf(stderr, "Usage: ./server ip port\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Now listening to the clients connections...\n");
        start_listening(argv[1], atoi(argv[2]));

    }
    return -1;
}
