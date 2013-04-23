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

// modif
#define CAFILE "root.pem"
#define CADIR NULL
#define CERTFILE "server.pem"
#define KEYFILE "server_key.pem"
// fin modif


SocketTCP *listen_socket;
pthread_mutex_t mutex;

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

// modif

SSL_CTX *setup_server_ctx(void) {
    ctx = SSL_CTX_new(SSLv23_method());
    if (SSL_CTX_load_verify_locations(ctx, CAFILE, CADIR) != 1)
        fprintf(stderr, "Error loading CA file or directory\n");
        //int_error("Error loading CA file and/or directory");
    if (SSL_CTX_set_default_verify_paths(ctx) != 1)
        fprintf(stderr, "Error loading CA file or directory\n");
    if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
        fprintf(stderr, "Error loading certificate from file\n");
    if (SSL_CTX_use_PrivateKey_file(ctx, KEYFILE, SSL_FILETYPE_PEM) != 1)
        fprintf(stderr, "Error loading private key from file\n");

    SSL_CTX_set_verify(ctx,
    SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
    SSL_CTX_set_verify_depth(ctx, 4);
    return ctx;
}

// fin modif

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
        SSL_CTX_load_verify_locations(ctx, CertFile, ".");
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
        SSL_CTX_set_verify_depth(ctx,1);

        //SSL_connect(client_ssl);
        if (SSL_get_peer_certificate(client_ssl) != NULL) {
            if(SSL_get_verify_result(client_ssl) == X509_V_OK) {
                printf("client verification with SSL_get_verify_result() succeeded.\n");
            }                
            else {
                printf("client verification with SSL_get_verify_result() failed.\n");
                exit(1);
            }
        } else {
            printf("the peer certificate was not presented.\n");
        }

       printf ("Before while\n");
        while (1) {
            // TODO attention code de test
            bytes = SSL_read(client_ssl, (void *) &buffer, sizeof(message)); /* get request */
	    //            printf ("SSL_read : %d bytes\n", bytes);
            if (bytes > 0) {
	      printf ("SSL_read : %d bytes\n", bytes);
	      printf ("Before ssl_mutex: buffer.content: %s\n",buffer.sender);
                //pthread_mutex_lock(&mutex);
		printf ("After ssl_mutex\n");
                switch (buffer.code) {
                case CREATE_ROOM_SEC:
                    //TODO
                    break;

                case JOIN_ROOM_SEC:
                    //TODO
                    break;

                case QUIT_ROOM_SEC:
                    //TODO
                    break;

                case DELETE_ROOM_SEC:
                    //TODO
                    break;

                case DISCONNECT_SEC:
                    printf("Disconnection\n");
                    response.code = DISCONNECT;
                    SSL_write(client_ssl, &response, sizeof(message));
                    pthread_mutex_unlock(&mutex);
                    SSL_CTX_free(ctx);
                    closeSocketTCP(s);
                    change_status (buffer.sender);
                    pthread_exit(0);
                    break;

                case CONNECT_SEC:
		          printf("BEGIN connect_sec\n");
                  int status = is_connected (buffer.sender);
                  switch (status) {
                    case 1:
                        response.code = KO;
                        strcpy (response.content, "you are already connected!\n");
                        printf("You are already connected\n");
                        break;
                    case -1:
                        if (check_user(buffer.sender) == 1) {
                            add_user (buffer.sender);
                        }
                        change_status(buffer.sender);
                        printf("successful connection : %s\n", buffer.sender);
                        u = (user_sec) malloc(sizeof(struct USER_SEC));
                        strcpy(u->name, buffer.sender);
                        u->ssl = client_ssl;
                        response.code = CONNECT_SEC;
                        break;
                    default:
                        // pas possible
                        break;
                  }
		      break;
                }
                SSL_write(client_ssl, &response, sizeof (message)); /* send reply */
            } else if (bytes <= 0) {
                change_status(buffer.sender);
                ERR_print_errors_fp(stderr);
                printf ("Error: the connection with the client just stopped (ServerSec)\n");
                pthread_mutex_unlock(&mutex);
                closeSocketTCP(s);
                pthread_exit(0);
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

    init_OpenSSL();
    //seed_prng();
    //ctx = initialize_ctx (CertFile, KeyFile, "bavardage");

    ctx = setup_server_ctx();   

    ssl = SSL_new(ctx);
    sbio = BIO_new_socket(listen_socket->socket, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    (void) signal(SIGINT, my_sigaction);
    pthread_mutex_init(&mutex, NULL);
    int err;
    while (1) {
        client = acceptSocketTCP(listen_socket);
        // modif
        if ((err = post_connection_check(ssl, "localhost")) != X509_V_OK) {
            fprintf(stderr, "-Error: peer certificate: %s\n", X509_verify_cert_error_string(err));
            //int_error("Error checking SSL object after connection");
        }
        // fin modif
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
