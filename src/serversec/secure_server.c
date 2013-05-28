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
#include <openssl/rand.h>
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
room_map server_room_map;
char *home_room="accueil";
char *home_room_sec = "accueil_sec";

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

SSL_CTX *setup_server_ctx (void) {
    ctx = SSL_CTX_new (SSLv23_method ());
    if (SSL_CTX_load_verify_locations (ctx, CAFILE, CADIR) != 1)
        fprintf (stderr, "Error loading CA file or directory\n");
    if (SSL_CTX_set_default_verify_paths (ctx) != 1)
        fprintf (stderr, "Error loading CA file or directory\n");
    if (SSL_CTX_use_certificate_chain_file (ctx, CERTFILE) != 1)
        fprintf (stderr, "Error loading certificate from file\n");
    if (SSL_CTX_use_PrivateKey_file (ctx, KEYFILE, SSL_FILETYPE_PEM) != 1)
        fprintf (stderr, "Error loading private key from file\n");

    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                        verify_callback);

    SSL_CTX_set_verify_depth (ctx, 1);
    return ctx;
}


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

unsigned char *spc_rand(size_t l) {
    unsigned char *buf;
    buf = (unsigned char *) malloc (l);
    RAND_add ("/dev/urandom", 10, 1.0);
    if (!RAND_bytes (buf, l)) {
        fprintf (stderr, "The PRNG is not seeded!\n");
        abort ();
    }
    return buf;
}

void randomString (char *str, size_t n) {
    RAND_add("/dev/urandom", 10, 1.0);
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
    RAND_add("/dev/urandom", 10, 1.0);
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

unsigned char *gen_master_key () {
    return spc_rand (32);
}

unsigned char *gen_hash_key () {
    return spc_rand (16);
}

keys gen_keys () {
    keys k = malloc (sizeof (struct KEYS));
    memcpy (k->master_key, gen_master_key (), 32);
    memcpy (k->hash_key, gen_hash_key (), 16);
    return k;
}

void *handle_connexion(void *param) {
    SocketTCP *s = (SocketTCP *) param;
    SSL *client_ssl;
    client_ssl = SSL_new(ctx);              /* get new SSL state with context */
    SSL_set_fd(client_ssl, s->socket);      /* set connection socket to SSL state */

    X509 *cert;
    char data[256];

    message buffer, response;
    int bytes = 0;
    user u,use;
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

   
        while (1) {
            // TODO attention code de test
            bytes = SSL_read(client_ssl, (void *) &buffer, sizeof(message)); /* get request */
            //            printf ("SSL_read : %d bytes\n", bytes);
            if (bytes > 0) {
                switch (buffer.code) {

                case CREATE_ROOM_SEC:
					printf ("Create room sec %s\n", buffer.content);
                    if (is_room_used(buffer.content)) {

                        response.code = CREATE_ROOM_KO;

                        strcpy(response.content,
                               buffer.content);
                        printf ("The room %s already exists\n", buffer.content);
                    } else {
                    
                        //~ randomString(key_data,(sizeof key_data)-1);
                        //~ keyiv = malloc(sizeof(struct KEY_IV));
                        //~ gen_keyiv(keyiv, (unsigned char *)key_data, sizeof(key_data));

                        keys k = gen_keys ();
                        add_room(buffer.content, u);
                        add_user_in_room(u, buffer.content);

                        response.code = CREATE_ROOM_SEC;
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);

                        strcat(response.content, "|");
                        memcpy(response.content + strlen (buffer.content) + 1, k->master_key, 32);
                        memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
                        memcpy(response.content + strlen (buffer.content) + 34, k->hash_key, 16);
                        
                        printf("The secure room %s has been created\n", buffer.content);
                      
                    }

                    break;
                case EJECT_FROM_ROOM_SEC:
                    if (u == get_admin (buffer.content)) {
                        printf("EJECT_FROM_ROOM_SEC -  buffer.receiver: <%s>\n", buffer.receiver);
                        user claimer = get_user (buffer.receiver, server_user_map);
                        remove_user_from_room (claimer, buffer.content);
                        //~ randomString(key_data,(sizeof key_data)-1);
                        //~ keyiv = malloc (sizeof(struct KEY_IV));
                        //~ gen_keyiv(keyiv,(unsigned char *)key_data, sizeof(key_data));

                        keys k = gen_keys ();
                        response.code = QUIT_ROOM_SEC;
                        
                        strcpy(response.sender, buffer.sender);
                        strcpy(response.content, buffer.content);
                        
                        SSL_write (claimer->ssl, &response, sizeof(message));

                        response.code = REFRESH_KEYIV;
                        strcat(response.content, "|");
                        memcpy(response.content + strlen (buffer.content) + 1, k->master_key, 32);
                        memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
                        memcpy(response.content + strlen (buffer.content) + 34, k->hash_key, 16);
                        
                        user_list l = get_users(buffer.content);
                        user_list t;
                        for (t = l; t != NULL; t = t->next) {
                            SSL_write(t->current_user->ssl,
                                      &response, sizeof(message));
                        }
                        free(k);
                        response.code = OK;
                    } else {
                        response.code = KO;
                        strcpy (response.content, "You're not admin of the room ");
                        strcat (response.content, buffer.content);
                    }
                    break;
                case ACCEPT_JOIN_ROOM_SEC:
                    if (u == get_admin (buffer.content)) {
                        user claimer = get_user (buffer.receiver, server_user_map);
                        if (claimer == NULL) {
                            response.code = KO;
                            strcpy (response.content, "This user is not connected ");
                            strcat (response.content, buffer.receiver);
                        } else {
                            add_user_in_room (claimer, buffer.content);
                            
                            keys k = gen_keys ();
                            response.code = CREATE_ROOM_SEC;
                            
                            strcpy(response.sender, buffer.sender);
                            strcpy(response.content, buffer.content);
                            strcat(response.content, "|");
                            memcpy(response.content + strlen (buffer.content) + 1, k->master_key, 32);
                            memcpy(response.content + strlen (buffer.content) + 33, "|", 32);
                            memcpy(response.content + strlen (buffer.content) + 34, k->hash_key, 16);

                            SSL_write (claimer->ssl, &response, sizeof(message));


                            response.code = JOIN_ROOM_SEC;

                            user_list l = get_users(buffer.content);
                            user_list t;
                            for (t = l; t != NULL; t = t->next) {
                                SSL_write(t->current_user->ssl,
                                          &response, sizeof(message));
                            }
                            free(k);
                            response.code = OK;
                        }
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
                    if (!is_room_used (buffer.content)) {
                        strcpy (response.content, "The room does not exist");
                        response.code = JOIN_ROOM_KO;
                    } else if (is_user_in_room (u, buffer.content)) {
                        strcpy (response.content, "You're already in this room");
                        response.code = JOIN_ROOM_KO;
                    } else {
                        // Ask for invitation to the admin
                        user admin = get_admin (buffer.content);
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
                        response.code = QUIT_ROOM_SEC;                    
                        strcpy (response.content, strtok(buffer.content, "|"));         
                        break;
                        
                    } else if (!is_user_in_room (u, buffer.content)) {
                        response.code = QUIT_ROOM_SEC_KO;
                        strcpy (response.content, "You are not in this room");
                        break;
                    }
                   
                case DELETE_ROOM_SEC:
                    if (u != get_admin (buffer.content)) {
                        response.code = DELETE_ROOM_KO;
                        strcpy (response.content,
                                "You're not admin, you can't delete this room");
                    } else {
                        delete_room (buffer.content);
                        response.code = OK;
                    }
                    break;
                case MP_SEC:
					if (get_user(buffer.receiver,server_user_map)!=NULL) {
					
                    keys k = gen_keys ();
					strcpy(response.content, buffer.receiver);
					strcat(response.content, "|");
					memcpy(response.content + strlen (buffer.receiver) + 1, k->master_key, 32);
					memcpy(response.content + strlen (buffer.receiver) + 33, "|", 32);
					memcpy(response.content + strlen (buffer.receiver) + 34, k->hash_key, 16);					
					response.code=MP_SEC_OK;
					use=get_user(buffer.receiver,server_user_map);
					SSL_write(u->ssl,&response, sizeof (message));
					strcpy(response.content, buffer.sender);
					strcat(response.content, "|");
					memcpy(response.content + strlen (buffer.sender) + 1, k->master_key, 32);
					memcpy(response.content + strlen (buffer.sender) + 33, "|", 32);
					memcpy(response.content + strlen (buffer.sender) + 34, k->hash_key, 16);					
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
                case DISCONNECT_SEC: 
                    printf("Disconnection from secure server\n");
                   // printf("first_connection: <%d>\n", first_connection);
                    if (first_connection == 1) {
                        delete_user(buffer.sender); 
                    }
                    if (u != NULL) {
                        room_list l;
                        for (l = get_user_rooms (u); l != NULL;
                             l = l->next) {
                            if (u == get_admin (l->current->name)) {
                                remove_room (l->current->name);
                            } else {
                                quit_room (u, l->current->name);
                                //remove_user_from_room (u, l->current->name);
                            }
                        }
                        remove_user (u, server_user_map);
                        free (u);//DISCONNECT_SEC

                    }
                    response.code = DISCONNECT_SEC;
                    SSL_write(client_ssl, &response, sizeof(message));                
                    pthread_mutex_unlock(&mutex);
                    SSL_CTX_free(ctx);
                    SSL_free(client_ssl);
                    //closeSocketTCP(s);
                    change_status (buffer.sender);
                    pthread_exit(0);
                    break;

				case DEL_ACCOUNT_SEC:					
					if (is_connected(buffer.sender, data) == -1) {
						response.code = KO;
						strcpy (response.content, "bad user / password\n");
					} else if (is_connected(u->name, data) == 1) {
                        if (delete_user(u->name) == -1) {
                            printf ("you haven't been deleted!!\n");
                            break;
                        } else {
                            response.code = DEL_ACCOUNT_SEC;
                            strcpy (response.content, "you have been deleted\n");
                        }
                    }
                    break;
                case CONNECT_OK:                 
                    response.code = CONNECT_SEC_OK;
                    first_connection = 0;
                    break;
                case CONNECT_SEC:
                    printf(" ");
                    int status = is_connected (buffer.sender, data);          
                    switch (status) {
                    case -1:
                        response.code = CONNECT_SEC_KO;
                        strcpy (response.content, "you are already connected!\n");
                        break;
                    case 1:                     

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
    message m;
    remove_user_from_room(u, room_name);
    
    keys k = gen_keys ();
    m.code = QUIT_ROOM;
    strcpy(m.sender, u->name);
    strcpy(m.content, room_name);
    strcat(room_name, "|");
    memcpy(room_name + strlen (room_name) + 1, k->master_key, 32);
    memcpy(room_name + strlen (room_name) + 33, "|", 32);
    memcpy(room_name + strlen (room_name) + 34, k->hash_key, 16); 
    user_list l = get_users(room_name);
    user_list t;
    for (t = l; t != NULL; t = t->next) {
        SSL_write(t->current_user->ssl,
                  &m, sizeof(message));
    }

    return 0;
}


int delete_room (char *room_name) {
    user_list users = get_users(room_name);
    user_list t;
    // On demande à tous les clients connectés au salon de le supprimer
    message m;
    m.code = QUIT_ROOM_SEC;
    strcpy(m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        SSL_write(t->current_user->ssl,
                  (char *) &m, sizeof(message));
    }
    if (remove_room (room_name) == -1) {
        printf("erreur remove_room secure_server\n");
    }
    return 0;
}

int create_main_room() {
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
   
    ctx = setup_server_ctx();

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

int main (int argc, char *argv[]) {
    connect_server_database ("secureserver.db");

    if (argc < 3) {
        fprintf (stderr, "Usage: ./secure_server ip port\n");
        exit (EXIT_FAILURE);
    } else {
        printf ("Now listening to the clients connections...\n");
        start_listening (argv[1], atoi (argv[2]));

    }
    return -1;
}
