#include "secure_server.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include "../common/commonsec.h"
#include "../common_server/mysqlite.h"
#include "../common/room_manager.h"
#include "../common/user_manager.h"

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
#include <stdint.h>

// modif
#define CAFILE "root.pem"
#define CADIR NULL
#define CERTFILE "secserv_certif.pem"
#define KEYFILE "secserv_key.pem"
// fin modif

SocketTCP *listen_socket;
pthread_mutex_t mutex;
user_map server_user_map;
char *home_room = "accueil";

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

// modif

SSL_CTX *setup_server_ctx (void) {
    ctx = SSL_CTX_new (SSLv23_method ());
    if (SSL_CTX_load_verify_locations (ctx, CAFILE, CADIR) != 1)
        fprintf (stderr, "Error loading CA file or directory\n");
    //int_error("Error loading CA file and/or directory");
    if (SSL_CTX_set_default_verify_paths (ctx) != 1)
        fprintf (stderr, "Error loading CA file or directory\n");
    if (SSL_CTX_use_certificate_chain_file (ctx, CERTFILE) != 1)
        fprintf (stderr, "Error loading certificate from file\n");
    if (SSL_CTX_use_PrivateKey_file (ctx, KEYFILE, SSL_FILETYPE_PEM) != 1)
        fprintf (stderr, "Error loading private key from file\n");

    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                        verify_callback);
    SSL_CTX_set_verify_depth (ctx, 4);
    return ctx;
}

// fin modif

void my_sigaction (int s) {
    switch (s) {
    case SIGINT:
        closeSocketTCP (listen_socket);
        exit (0);
        break;
    default:
        break;
    }
}

void randomString (char *str, size_t n) {
    srand (time (NULL));
    char c;
    int i;
    for (i = 0; i < n; i++) {
        do {
            c = rand () % 'z';
        } while (!(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')));
        str[i] = c;
    }
    str[i] = 0;
}


unsigned int *randomInt() {
    unsigned int *c;
    c = (unsigned int *) malloc (2 * sizeof (unsigned int));
    int i, v;
    srand ((unsigned) time(NULL));
    for ( i = 0; i < 2; ++i) {
        do {
            v = rand() % 1000000;
        }
        while (!(10000 <= v && v <= 99999));
        c[i] = v;
    }
    return c;

}

void gen_keyiv(key_iv keyiv, unsigned char *key_data, int key_data_len) {
    int nrounds = 5;
    unsigned int *salt = malloc (sizeof (randomInt ()));
    salt = randomInt();
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(),(unsigned char *)salt,(unsigned char *)key_data, strlen((char *)key_data), nrounds, keyiv->key, keyiv->iv);

}

void *handle_connexion(void *param) {
    printf ("BEGIN handle_connexion\n");
    SocketTCP *s = (SocketTCP *) param;
    SSL *client_ssl;
    client_ssl = SSL_new(ctx);              /* get new SSL state with context */
    SSL_set_fd(client_ssl, s->socket);      /* set connection socket to SSL state */

    message buffer, response;
    int bytes = 0;
    user u;
    key_iv keyiv;

    char key_data[KEY_DATA_SIZE];


    if (SSL_accept(client_ssl) <= 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        SSL_CTX_load_verify_locations(ctx, CERTFILE, ".");
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
                printf("buffer.code = %d\n", buffer.code);
                switch (buffer.code) {
                case CREATE_ROOM_SEC:
                    printf ("Create room : %s\n", buffer.content);
                    if (is_room_used(buffer.content)) {
                        response.code = CREATE_ROOM_SEC_KO;
                        strcpy(response.content,
                               "This room name is already in use");
                        printf ("Room already in user\n");
                    } else {
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc(sizeof(struct KEY_IV));
                        gen_keyiv(keyiv, (unsigned char *)key_data, sizeof(key_data));
                        add_room(buffer.content, u);
                        add_user_in_room(u, buffer.content);

                        response.code = CREATE_ROOM;
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);
                        SSL_write(u->ssl, (char *) &response, sizeof (message));
                          
                        response.code = ADMIN;
                        sprintf(response.content, "|%s|%s",keyiv->key,keyiv->iv);
                       
                        strcpy (response.sender, u->name);
                        SSL_write(u->ssl, (char *) &response, sizeof (message));

                        response.code = OK;
                        free(keyiv);
                    }


                    break;

                case JOIN_ROOM_SEC:
                    printf ("Join room : %s\n", buffer.content);
                    if (!is_room_used (buffer.content)) {
                        strcpy (response.content, "The room does not exist");
                        response.code = JOIN_ROOM_SEC_KO;
                    } else if (is_user_in_room (u, buffer.content)) {
                        strcpy (response.content, "You're already in this room");
                        response.code = JOIN_ROOM_SEC_KO;
                    } else {
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc(sizeof(struct KEY_IV));
                        gen_keyiv(keyiv,(unsigned char *)key_data, sizeof(key_data));
                        join_room (u, buffer.content);
                        strcpy(response.content, buffer.content);
                        sprintf(response.content, "|%s|%s",keyiv->key,keyiv->iv);
                        response.code = OK;
                        user_list l = get_users(buffer.receiver);
                        user_list t;
                        for (t = l; t != NULL; t = t->next) {
                            SSL_write(t->current_user->ssl,
                                      &response, sizeof(message));
                        }
                        free(keyiv);

                    }
                    break;
                case QUIT_ROOM_SEC:
                    if (!is_room_used (buffer.content)) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "This room does not exist");
                        break;
                    } else if (strcmp (home_room, buffer.content) == 0) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "You cannot leave the home room");
                        break;
                    } else if (u != get_admin(buffer.content)) {
                        quit_room (u, buffer.content);
                        remove_user_from_room(u, buffer.content);
                        printf("User successfully deleted\n");
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc(sizeof(struct KEY_IV));
                        gen_keyiv(keyiv,(unsigned char *)key_data, sizeof(key_data));
                        sprintf(response.content, "|%s|%s",keyiv->key,keyiv->iv);
                        response.code = OK;
                        user_list l = get_users(buffer.receiver);
                        user_list t;
                        for (t = l; t != NULL; t = t->next) {
                            SSL_write(t->current_user->ssl,
                                      &response, sizeof(message));
                        }

                        response.code = DELETE_ROOM;
                        strcpy (response.content, buffer.content);
                        free(keyiv);
                        break;
                    } else if (!is_user_in_room (u, buffer.content)) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "You are not in this room");
                        break;
                    }


                case DELETE_ROOM_SEC:
                    //TODO
                    break;

                case DISCONNECT_SEC:
                    printf("Disconnection from secure server\n");
                    response.code = DISCONNECT_SEC;
                    SSL_write(client_ssl, &response, sizeof(message));
                    printf("SSL_write secure server\n");
                    pthread_mutex_unlock(&mutex);
                    SSL_CTX_free(ctx);
                    SSL_free(client_ssl);
                    //closeSocketTCP(s);
                    change_status (buffer.sender);
                    pthread_exit(0);
                    break;

                case CONNECT_SEC:
                    printf("BEGIN connect_sec\n");
                    uint8_t *u8bufcontent;
					u8bufcontent = (uint8_t *) buffer.content;
                    int status = is_connected (buffer.sender, u8bufcontent);
                    printf("AFTER is connected\n");
                    switch (status) {
                    case -1:
                        response.code = CONNECT_SEC_KO;
                        strcpy (response.content, "you are already connected!\n");
                        printf("You are already connected\n");
                        break;
                    case 1:
                        printf("DEBUT CASE 1\n");
                        uint8_t *u8bufcontent;
						u8bufcontent = (uint8_t *) buffer.content;
                        if (check_user(buffer.sender, u8bufcontent) == 1) {
                            add_user_db (buffer.sender, u8bufcontent);
                        } else if (check_user(buffer.sender, u8bufcontent) == -1) {
                            fprintf(stderr, "incorrect login / password\n");
                            response.code = CONNECT_SEC_KO;
			    strcpy (response.content, "Incorrect password\n");
                            response.code = KO;
                            break;
                        }
                        printf("AFTER CHECK_USER\n");
                        change_status(buffer.sender);
                        printf("successful connection : %s\n", buffer.sender);
                        u = (user) malloc(sizeof(struct USER));
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
    return NULL;
}

int is_login_valid(char *login) {
    int i = 0;
    char c = login[0];
    while (c != '\0') {
        if (!('0' <= c && c <= '9') && !('a' <= c && c <= 'z')
            && !('A' <= c && c <= 'Z')) {
            return 0;
        }
        c = login[++i];
    }
    return 1;
}

int clear_message(message *m) {
    strcpy(m->sender, "");
    strcpy(m->content, "");
    strcpy(m->receiver, "");
    m->code = -1;
    return 0;
}

int join_room (user u, char *room_name) {
    user_list users = get_users(room_name);
    user_list t, l;
    message m;

    m.code = NEW_USER;
    strcpy(m.sender, u->name);
    strcpy(m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        SSL_write(t->current_user->ssl, (char *) &m,
                  sizeof(message));
    }

    strcpy(m.content, room_name);
    m.code = CREATE_ROOM;
    SSL_write (u->ssl, (char *) &m, sizeof (message));
    add_user_in_room(u, room_name);

    if (get_admin (room_name) != NULL) {
        m.code = ADMIN;
        strcpy(m.content, room_name);
        strcpy (m.sender, get_admin (room_name)->name);
        SSL_write (u->ssl, (char *) &m, sizeof (message));
    }

    m.code = ADD_USER;
    strcpy(m.content, room_name);
    users = get_users(room_name);
    for (l = users; l != NULL; l = l->next) {
        if (l->current_user != get_admin (room_name)) {
            strcpy(m.sender, l->current_user->name);
            SSL_write(u->ssl, (char *) &m,
                      sizeof(message));
        }
    }
    return 0;
}

int quit_room (user u, char *room_name) {
    user_list users = get_users(room_name);
    user_list t;
    message m;
    m.code = RM_USER;
    strcpy(m.sender, u->name);
    strcpy(m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        SSL_write(t->current_user->ssl, (char *) &m,
                  sizeof(message));
    }
    return 0;
}


int delete_room (char *room_name) {
    user_list users = get_users(room_name);
    user_list t;
    // On demande à tous les clients connectés au salon de le supprimer
    message m;
    m.code = DELETE_ROOM;
    strcpy(m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        SSL_write(t->current_user->ssl,
                  (char *) &m, sizeof(message));
    }
    remove_room (room_name);
    return 0;
}

int create_main_room() {
    printf("Server room created\n");
    init_rooms();
    add_room(home_room, NULL);
    server_user_map = (user_map) malloc(HASH_USER_MAP_SIZE * sizeof(user_list));
    return 0;
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

int start_listening (const char *addr, int port) {
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
    //int err;
    while (1) {
        client = acceptSocketTCP(listen_socket);
        // modif a refaire
        /*if ((err = post_connection_check(ssl, "localhost")) != X509_V_OK) {
          fprintf(stderr, "-Error: peer certificate: %s\n", X509_verify_cert_error_string(err));
          //int_error("Error checking SSL object after connection");
          }*/
        // fin modif
        printf("New connection\n");
        new_thread(client);
    }
    return -1;
}

int main (int argc, char *argv[]) { 
    connect_server_database ("secureserver.db");
    if (argc < 3) {
        fprintf (stderr, "Usage: ./server ip port\n");
        exit (EXIT_FAILURE);
    } else {
        printf ("Now listening to the clients connections...\n");
        start_listening (argv[1], atoi (argv[2]));

    }
    return -1;
}
