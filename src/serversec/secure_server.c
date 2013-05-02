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
<<<<<<< Updated upstream

char *home_room = "accueil";
=======
room_map server_room_map;
char *home_room="accueil";
char *home_room_sec = "accueil_sec";
>>>>>>> Stashed changes

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

    X509 *cert;
    char data[256];

    message buffer, response;
    int bytes = 0;
    user u,use;
    key_iv keyiv;
    char key_data[KEY_DATA_SIZE];

    int first_connection = 0;

    if (SSL_accept(client_ssl) <= 0) {     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    } else {
        SSL_CTX_load_verify_locations(ctx, CERTFILE, ".");
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0);
        SSL_CTX_set_verify_depth(ctx,1);

        //SSL_connect(client_ssl);
        if ((cert = SSL_get_peer_certificate(client_ssl)) != NULL) {
            if(SSL_get_verify_result(client_ssl) == X509_V_OK) {
                printf("client verification with SSL_get_verify_result() succeeded.\n");
                X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
                fprintf(stderr, "subject = %s\n", data);
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
                printf ("Before ssl_mutex: buffer.content: %s\n",buffer.content);
                //pthread_mutex_lock(&mutex);
                printf ("After ssl_mutex\n");
                printf("buffer.code = %d\n", buffer.code);
                switch (buffer.code) {

                case CREATE_ROOM_SEC:
                    printf ("Create room : <%s>\n", buffer.content);
                    if (is_room_used(buffer.content)) {

                        printf("debut is_room_used\n");
                        response.code = CREATE_ROOM_KO;

                        strcpy(response.content,
                               buffer.content);
                        printf ("Room already in user\n");
                    } else {
                        printf("Debut create room sec\n");
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc(sizeof(struct KEY_IV));
                        gen_keyiv(keyiv, (unsigned char *)key_data, sizeof(key_data));
                        printf ("generated key = %s, iv = %s\n", keyiv->key, keyiv->iv);
                        add_room(buffer.content, u);
                        add_user_in_room(u, buffer.content);

                        response.code = CREATE_ROOM_SEC;
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);

                        strcat(response.content, "|");
                        memcpy(response.content + strlen (buffer.content) + 1, keyiv->key, 32);
                        memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
                        memcpy(response.content + strlen (buffer.content) + 34, keyiv->iv, 32);

                        //free(keyiv);
                        printf("Fin create room sec\n");
                    }

                    break;

                case ACCEPT_JOIN_ROOM_SEC:
                    if (u == get_admin (buffer.content)) {
                        user claimer = get_user (buffer.receiver, server_user_map);
                        add_user_in_room (claimer, buffer.content);
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc (sizeof(struct KEY_IV));
                        gen_keyiv(keyiv,(unsigned char *)key_data, sizeof(key_data));

			response.code = CREATE_ROOM_SEC;
                        
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);
                        strcat(response.content, "|");
                        memcpy(response.content + strlen (buffer.content) + 1, keyiv->key, 32);
                        memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
                        memcpy(response.content + strlen (buffer.content) + 34, keyiv->iv, 32);

			SSL_write (claimer->ssl, &response, sizeof(message));


			response.code = JOIN_ROOM_SEC;

                        user_list l = get_users(buffer.content);
                        user_list t;
                        for (t = l; t != NULL; t = t->next) {
                            SSL_write(t->current_user->ssl,
                                      &response, sizeof(message));
                        }
                        free(keyiv);
			response.code = OK;
                    } else {
			response.code = KO;
			strcpy (response.content, "You're not admin of the room ");
			strcat (response.content, buffer.content);
		    }
                    break;
		
		case REFUSE_JOIN_ROOM_SEC:
		    if (u == get_admin (buffer.content)) {
                        user claimer = get_user (buffer.receiver, server_user_map);

			response.code = JOIN_ROOM_SEC_KO;
			strcpy (response.content, buffer.content);

			SSL_write (claimer->ssl, &response, sizeof(message));

			response.code = OK;
                    } else {
			response.code = KO;
			strcpy (response.content, "You're not admin of the room ");
			strcat (response.content, buffer.content);
		    }
                    break;

                case JOIN_ROOM_SEC:
                    printf ("secure_server.c: JOIN_ROOM_SEC: %s\n", buffer.content);
                    if (!is_room_used (buffer.content)) {
                        strcpy (response.content, "The room does not exist");
                        response.code = JOIN_ROOM_KO;
                    } else if (is_user_in_room (u, buffer.content)) {
                        strcpy (response.content, "You're already in this room");
                        response.code = JOIN_ROOM_KO;
                    } else {
                        // Ask for invitation to the admin
                        user admin = get_admin (buffer.content);
			printf ("> asking the admin for request : %s, admin->ssl == NULL ? %d\n", admin->name, (admin->ssl == NULL));
                        response.code = ASK_JOIN_ROOM_SEC;
                        strcpy (response.sender, u->name);
                        strcpy (response.content, buffer.content);
                        SSL_write (admin->ssl, &response, sizeof (message));
                        response.code = OK;
                    }
                    break;

                case QUIT_ROOM_SEC:
                    if (!is_room_used (buffer.content)) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "This room does not exist");
                        break;
                    } else if (strcmp (home_room_sec, buffer.content) == 0) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "You cannot leave the home room");
                        break;
                    } else if (u != get_admin(buffer.content)) {
                        quit_room (u, buffer.content);
                        remove_user_from_room(u, buffer.content);
                        printf("User successfully deleted in room %s\n", buffer.content);
                        randomString(key_data,(sizeof key_data)-1);
                        keyiv = malloc(sizeof(struct KEY_IV));
                        gen_keyiv(keyiv,(unsigned char *)key_data, sizeof(key_data));
                        response.code = QUIT_ROOM;
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);
						strcat(response.content, "|");
						memcpy(response.content + strlen (buffer.content) + 1, keyiv->key, 32);
						memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
						memcpy(response.content + strlen (buffer.content) + 34, keyiv->iv, 32); 
                        user_list l = get_users(buffer.content);
                        user_list t;
                        for (t = l; t != NULL; t = t->next) {
                            SSL_write(t->current_user->ssl,
                                      &response, sizeof(message));
                        }

                        response.code = DELETE_ROOM;
                        strcpy (response.content, buffer.content);
                        //free(keyiv);
                        break;
                    } else if (!is_user_in_room (u, buffer.content)) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "You are not in this room");
                        break;
                    }
                case MP_SEC:
					if (get_user(buffer.receiver,server_user_map)!=NULL) {
					randomString(key_data,(sizeof key_data)-1);
					keyiv = malloc (sizeof(struct KEY_IV));
					gen_keyiv(keyiv,(unsigned char *) key_data, sizeof(key_data));
					strcpy(response.content, buffer.receiver);
					strcat(response.content, "|");
					memcpy(response.content + strlen (buffer.receiver) + 1, keyiv->key, 32);
					memcpy(response.content + strlen (buffer.receiver) + 33, "|", 32);
					memcpy(response.content + strlen (buffer.receiver) + 34, keyiv->iv, 32);					
					response.code=MP_SEC_OK;
					use=get_user(buffer.receiver,server_user_map);
					SSL_write(u->ssl,&response, sizeof (message));
					strcpy(response.content, buffer.sender);
					strcat(response.content, "|");
					memcpy(response.content + strlen (buffer.sender) + 1, keyiv->key, 32);
					memcpy(response.content + strlen (buffer.sender) + 33, "|", 32);
					memcpy(response.content + strlen (buffer.sender) + 34, keyiv->iv, 32);					
					SSL_write(use->ssl,&response, sizeof (message));
					strcpy(response.content,buffer.content);
					strcpy(response.receiver,buffer.receiver);
					response.code=MP_SEC;
					}
					else
					{
						response.code=MP_SEC_KO;
					}
					break;

                case DELETE_ROOM_SEC:
                    //TODO
                    break;
                case DISCONNECT_SEC: 
                    printf("Disconnection from secure server\n");
                    printf("first_connection: <%d>\n", first_connection);
                    if (first_connection == 1) {
                        delete_user(buffer.sender); 
                    }
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

				case DEL_ACCOUNT_SEC:
					printf("serversec.c: buffer.sender = %s : buffer.content = %s\n", buffer.sender, buffer.content);
					printf("serversec.c: buffer.code = %d\n", buffer.code);
					if (is_connected(buffer.sender, data) == -1) {
						response.code = KO;
						strcpy (response.content, "bad user / password\n");
					} else if (is_connected(buffer.sender, data) == 1) {
						u = (user) malloc(sizeof(struct USER));
						strcpy(u->name, buffer.sender);
						//room_list p;
						//------------------------------------------------
						/*printf("disconnection from all current room...\n");
						for (p = server_room_map; p != NULL; p = p->next) {
							if (is_user_in_room (u, p->current->name))
								remove_user_from_room (u, p->current->name);
						}*/
						//------------------------------------------------

                        printf("DEL_ACCOUNT_SEC: buffer.sender: <%s> | buffer.content: <%s>\n", buffer.sender, buffer.content);
                        if (strcmp(buffer.sender, buffer.content) != 0)
                            printf("bad login, please retry...\n");
                        else {
                            if (delete_user(buffer.content) == -1) {
                                printf ("you haven't been deleted!!\n");
                                break;
                            } else {
                                response.code = DEL_ACCOUNT_SEC;
                                strcpy (response.content, "you have been deleted\n");
                            }
                        }
                    }
                    //TODO revoquer le certicat ??
                    break;
                case CONNECT_OK:
                    printf("--------------------------DEBUT CONNECT_SOK secure_server ------------------------\n");
                    response.code = CONNECT_SEC_OK;
                    first_connection = 0;
                    break;
                case CONNECT_SEC:
                    printf("--------------------------DEBUT CONNECT_SEC secure_server ------------------------\n");
                    printf("secure_server.c: handle_connexion: BEGIN connect_sec\n");
                    printf("serversec.c: buffer.sender = %s:\nbuffer.content = %s\n", buffer.sender, buffer.content);
                    printf("Data: <%s>\n", data);
                    int status = is_connected (buffer.sender, data);
                    printf("secure_server.c: handle_connexion: AFTER is connected\n");
                    switch (status) {
                    case -1:
                        response.code = CONNECT_SEC_KO;
                        strcpy (response.content, "you are already connected!\n");
                        printf("You are already connected\n");
                        break;
                    case 1:
                        printf("secure_server.c: handle_connexion: connect_sec: DEBUT CASE 1\n");

                        if (check_user(buffer.sender, data) == 1) {
                            if (check_certificate(data) == -1) {
                                fprintf(stderr, "This certificate is already in use\n");
                                strcpy(response.content, "This certificate is already in use");
                                response.code = CONNECT_SEC_KO;
                                break;
                            } else {
                                add_user_db (buffer.sender, data);
                                first_connection = 1;

                            }
                        } 

                        printf("secure_server.c: handle_connexion: connect_sec: AFTER CHECK_USER\n");
                        change_status(buffer.sender);
                        printf("successful connection : %s\n", buffer.sender);
                        u = (user) malloc(sizeof(struct USER));
                        strcpy(u->name, buffer.sender);
                        strcpy(response.sender, buffer.sender);
                        u->ssl = client_ssl;
			add_user (u, server_user_map);
                        response.code = CONNECT_SEC;
                        add_user (u, server_user_map);
                        join_room (u, home_room_sec);                        
                        
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
    add_room(home_room_sec, NULL);
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
    create_main_room();
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
