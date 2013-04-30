#include "libclientsec.h"
#include "libclient.h"
#include "../common/commonsec.h"
#include "../common/common.h"
#include "../common/room_manager.h"
#include "../common/room_manager_sec.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

#define CHATADDR "localhost"
#define CHATPORT 10000

#define SECADDR  "localhost"
#define SECPORT  11000

pthread_t thread_send, thread_recv, thread_recv_sec;
key_iv keyiv;
EVP_CIPHER_CTX en;
EVP_CIPHER_CTX de;

int leng;

void *traitement_send (void *param) {
    char mess[MAX_MESS_SIZE] = "";
    while (fgets (mess, MAX_MESS_SIZE, stdin) != NULL) {
        mess[strlen (mess) - 1] = '\0';
        int ret, ret_sec;
        char *error_mess;
        /*if (ret = send_message (mess, &error_mess) == -1) {
          pthread_exit (NULL);
          exit (EXIT_FAILURE);
          } else if (ret == -2) {
          fprintf (stderr, "Unkown command\n");
          } else if (ret == -3) {
          fprintf (stderr, "%s\n", error_mess);
          }*/

        if ((ret_sec = send_message_sec (mess, &error_mess)) == -1) {
            pthread_exit (NULL);
            exit (EXIT_FAILURE);
        } else if (ret_sec == -2) {
            fprintf (stderr, "Unkown command for secure server\n");
            if (ret = send_message (mess, &error_mess) == -1) {
                pthread_exit (NULL);
                exit (EXIT_FAILURE);
            } else if (ret == -2) {
                fprintf (stderr, "Unkown command\n");
            } else if (ret == -3) {
                fprintf (stderr, "%s\n", error_mess);
            }
        } else if (ret_sec == -3) {
            fprintf (stderr, "%s\n", error_mess);
        }
        if (ret_sec != -1) {
            printf("Clientsec-cli.c: traitement_send: send_message_sec fait : <%s>\n", mess);
        }
    }
    pthread_exit (0);
}


void *traitement_recv_sec (void *param) {

	message mess;	
	int lenght;
    unsigned char *plainmess;  
    char *room_name;
    key_iv keyiv;

    while (1) {
        printf("Clientsec-cli.c: recv: mess.code: <%d>\n", mess.code);
        printf("Clientsec-cli.c: recv: mess.content:<%s>\n", mess.content);
        if (receive_message_sec(&mess) == -1) {
            perror("SSL_read");
            pthread_detach(thread_recv_sec);
            pthread_detach(thread_send);
            exit(EXIT_FAILURE);
        }
        printf("mess.code: <%d>\n", mess.code);
        if (mess.code == KO) {
            printf("Error: %s\n", mess.content);
            continue;
        }


        char *res = NULL, conn[MAX_MESS_SIZE] = "";
        switch (mess.code) {
        case CONNECT_SEC:
            strcpy(conn, "/CONNECT ");
            printf("mess.sender: <%s>\n", mess.sender);
            strcat(conn, mess.sender);
            send_message (conn, NULL);
            printf("init_rooms: <%d>\n", init_rooms());
            break;

        case DISCONNECT:
            //disconnect ();
            printf ("You're now disconnected from the chat server\n");
            pthread_detach (thread_send);
            exit (0);

        case DISCONNECT_SEC:
            //disconnect_sec();
            printf ("You're now disconnected from the chat secure server\n");
            pthread_detach (thread_send);
            exit (0);

        case DEL_ACCOUNT_SEC:
            printf("your account %s have been deleted\n", mess.content);
            break;

        case OK:
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name =strdup(strtok(mess.content, "|"));
		    strcpy(keyiv->key, strtok(NULL, "|"));
		    strcpy(keyiv->iv, strtok(NULL, "|"));
		    set_keyiv_in_room(room_name, keyiv);
		    free(keyiv);
            if (strlen(mess.content) > 0) {
                printf("mess content = %s\n", mess.content);
            }
            break;

            /*  case MESSAGE:
                keyiv = get_keyiv_in_room(mess.receiver);
                aes_init(keyiv->key, keyiv->iv, &en, &de);
                lenght = strlen(mess.content) + 1;
                plainmess = aes_decrypt(&de, (unsigned char *)mess.content, &lenght);
                printf("[%s @ %s] %s\n", mess.sender, mess.receiver, plainmess);
                break;

                case MP:
                keyiv = get_keyiv_in_room(mess.receiver);
                aes_init(keyiv->key, keyiv->iv, &en, &de);
                lenght = strlen(mess.content) + 1;
                plainmess = aes_decrypt(&de, (unsigned char *)mess.content, &lenght);
                printf("[%s > %s] %s\n", mess.sender, mess.receiver, plainmess);
                break; */
            
        case CREATE_ROOM:
             printf("Création room réussie \n");
             add_room_client(mess.receiver);
             break;                       

        case NEW_USER:
            set_keyiv_in_room(mess.content, keyiv);
            printf("The user %s joined the room %s\n", mess.sender, mess.content);
            break;
            
        case ADD_USER:
            set_keyiv_in_room(mess.content, keyiv);
            printf("USER %s in %s \n", mess.sender, mess.content);
            break;

        case DELETE_ROOM:
            printf("The room %s has been deleted\n", mess.content);
            break;

        default:
            break;

        }
    }
    pthread_exit (0);
}

void *traitement_recv (void *param) {
    message mess;
    int lenght;
    unsigned char *plainmess;
    key_iv keyiv;
    char text[MAX_MESS_SIZE] = " ";

    unsigned char *plaintext;
    unsigned char key[32], iv[32];
    unsigned char *keydata="test";
    unsigned int salt[] = {12345, 54321};

    while (1) {
        if (receive_message (&mess) < 0) {
            perror ("readSocketTCP");
            pthread_detach (thread_recv);
            pthread_detach (thread_send);
            exit (EXIT_FAILURE);
        }

        if (mess.code == KO) {
            printf ("Error: %s\n", mess.content);

            continue;
        }

        char *res = NULL;
        if (mess.code == DISCONNECT) {
            disconnect ();
            printf ("You're now disconnected from the chat server\n");
            pthread_detach (thread_send);
            exit (0);
        }
        switch (mess.code) {
        case OK:
            if (strlen (mess.content) > 0) {
                printf ("%s\n", mess.content);
            }

            break;

        case MESSAGE:
            if (get_keyiv_in_room(mess.receiver) == NULL) {
                printf ("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
            }
            else {
                keyiv = malloc(sizeof (struct KEY_IV));
                keyiv = get_keyiv_in_room(mess.receiver);
                aes_init(keyiv->key, keyiv->iv, &en, &de);
                lenght = MAX_CIPHERED_SIZE;
                plainmess = aes_decrypt(&de, (unsigned char *)mess.content, &lenght);
                printf("[%s @ %s] %s\n", mess.sender, mess.receiver, plainmess);
            }
            break;

        case MP:
            /*if (get_keyiv_in_room(mess.receiver) == NULL) {
              printf ("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
              }
              else {
              keyiv = malloc(sizeof (struct KEY_IV));
              keyiv = get_keyiv_in_room(mess.receiver);
              aes_init(keyiv->key, keyiv->iv, &en, &de);
              lenght = strlen(mess.content) + 1;
              plainmess = aes_decrypt(&de, (unsigned char *)mess.content, &lenght);
              printf("[%s > %s] %s\n", mess.sender, mess.receiver, plainmess);
              }*/
            lenght = MAX_CIPHERED_SIZE;
            EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha512(), (unsigned char*)&salt, keydata, strlen(keydata), 5, key, iv);
            EVP_CIPHER_CTX_init(&de);
            EVP_DecryptInit_ex(&de, EVP_aes_256_cbc(), NULL, key, iv);
            //            printf("le message chiffré est :%d\n",mess.content);
            plaintext= (char *)aes_decrypt(&de, mess.content, &lenght);
            printf ("[%s > %s] %s\n", mess.sender, mess.receiver, plaintext);
            break;
            break;


            /* case MESSAGE:

               printf ("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
               break;

               case MP:
               <<<<<<< Updated upstream
               printf ("[%s > %s] %s\n", mess.sender, mess.receiver, mess.content);
               break; */


        case NEW_USER:
            printf ("The user %s joined the room %s\n", mess.sender,
                    mess.content);
            break;
        case ADD_USER:
            printf ("USER %s in %s \n", mess.sender, mess.content);
            break;

        case DELETE_ROOM:
            printf ("The room %s has been deleted\n", mess.content);
            break;

        case CREATE_ROOM:
            strcpy(text, "/CREATE_ROOM_SEC ");
            strcat(text,mess.content);
            send_message_sec(text, NULL);
            break;

        case CREATE_ROOM_KO:
            printf ("Error: %s\n", mess.content);

        default:
            break;
        }

    }
    pthread_exit (0);
}

int start_communication () {
    pthread_create (&thread_recv_sec, NULL, traitement_recv_sec, NULL);
    pthread_create (&thread_recv, NULL, traitement_recv, NULL);
    pthread_create (&thread_send, NULL, traitement_send, NULL);

    pthread_join (thread_recv, NULL);
    pthread_join (thread_send, NULL);
    pthread_join (thread_recv_sec,NULL);

    return 0;
}

int main (int argc, char *argv[]) {
    init_OpenSSL ();
    if (argc == 1) {
        connect_with_authentication (CHATADDR, CHATPORT, SECADDR, SECPORT);
    } else if (argc < 5 || argc > 5) {
        fprintf (stderr,
                 "Usage: ./client ip_client port_client ip_server port_server\n");
        exit (EXIT_FAILURE);
    } else {
        connect_with_authentication (argv[1], argv[2], argv[3], argv[4]);
    }

    start_communication ();
    return 0;
}
