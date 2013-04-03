#include "secure_server.h"
#include "structures.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include "../common/commonsec.h"
#include "../common_server/mysqlite.h"



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

    message buffer, response;
    int bytes = 0;
    user_sec u;
    if (SSL_accept(client_ssl) <= 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        while (1) {
            // TODO attention code de test
            bytes = SSL_read(client_ssl, (void *) &buffer, sizeof(message)); /* get request */
            printf ("SSL_read : %d bytes\n", bytes);
            if (bytes > 0) {

                pthread_mutex_lock(&mutex);
                switch (buffer.code) {
                case CREATE_ROOM:
                    //TODO
                    break;

                case JOIN_ROOM:
                    //TODO
                    break;

                case QUIT_ROOM:
                    //TODO
                    break;

                case DELETE_ROOM:
                    //TODO
                    break;

                case DISCONNECT:
                    printf("Disconnection\n");
                    response.code = DISCONNECT;
                    SSL_write(client_ssl, &response, sizeof(message));
                    pthread_mutex_unlock(&mutex);
                    closeSocketTCP(s);
                    pthread_exit(0);
                    break;

                case CONNECT:
                    if (check_user (buffer.sender) == -1) {
                        response.code = KO;
                        strcpy (response.content, "Cannot join the server with this login");
                    } else {
                        add_user (buffer.sender);
                        printf("successful connection : %s\n",
                               buffer.sender);
                        response.code = CONNECT;

                        u = (user_sec) malloc(sizeof(struct USER_SEC));
                        strcpy(u->name, buffer.sender);
                        u->ssl = client_ssl;
                    }

                    break;
                }
                SSL_write(ssl, &response, sizeof (message)); /* send reply */
            } else {
                ERR_print_errors_fp(stderr);
            }
        }
    }

    printf ("END handle_connexion\n");
    return -1;
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
    connect_server_database ("secureserver.db");
    if (argc < 3) {
        fprintf(stderr, "Usage: ./server ip port\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Now listening to the clients connections...\n");
        start_listening(argv[1], atoi(argv[2]));

    }
    return -1;
}
