#include "secure_server.h"
#include "../common/SocketTCP.h"

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

SSL_METHOD *method;   /* create server instance */
SSL_CTX *ctx;         /* create context */


int setup_ctx () {
    method = SSLv3_server_method();
    ctx = SSL_CTX_new(method);
    OpenSSL_add_all_algorithms();   /* load & register cryptos */
    SSL_load_error_strings();
    /* set the local certificate from CertFile */
    SSL_CTX_use_certificate_file (ctx, CertFile, SSL_FILETYPE_PEM);
    /* set the private key from KeyFile */
    SSL_CTX_use_PrivateKey_file (ctx, KeyFile, SSL_FILETYPE_PEM);
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx))
	abort();
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
    SocketTCP *s = (SocketTCP *) param;
    SSL *ssl = SSL_new(ctx);  /* get new SSL state with context */
    SSL_set_fd(ssl, s->socket);    /* set connection to SSL state */
    SSL_accept(ssl);           /* start the handshaking */
    while (1) {
        char *buf;
        int ret = SSL_read (ssl, buf, sizeof (buf));
        if (ret > 0) {
            printf ("Message reçu = %s\n", buf);
            break;
        }
    }
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
    if (argc < 3) {
        fprintf(stderr, "Usage: ./server ip port\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Now listening to the clients connections...\n");
        start_listening(argv[1], atoi(argv[2]));

    }
    return -1;
}
