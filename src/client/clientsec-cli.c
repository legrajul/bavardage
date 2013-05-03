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
//key_iv keyiv;

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
            fprintf (stderr, "Unkown command for secure server message = %s\n",mess);
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
    char text[MAX_MESS_SIZE] = " ";
    message mess;
    int lenght;
    char *plainmess;
    char *room_name;
    char *ciphermess;
    key_iv keyiv;
    key_iv ki;

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
            printf("--------------------------DEBUT CONNECT_SEC clientsec-cli ------------------------\n");
            strcpy(conn, "/CONNECT ");
            printf("mess.sender: <%s>\n", mess.sender);
            strcat(conn, mess.sender);
            send_message (conn, NULL);
            printf("init_rooms: <%d>\n", init_rooms());
            break;
        case CONNECT_SEC_OK:
            printf("--------------------------DEBUT CONNECT_SEC_OK clientsec-cli ------------------------\n");
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
            pthread_detach (thread_send);
            exit (0);
            break;

        case OK:
            break;
        case REFRESH_KEYIV:
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name =strdup(strtok(mess.content, "|"));
            memcpy (keyiv->key, mess.content + strlen (room_name) + 1, 32);
            memcpy (keyiv->iv, mess.content + strlen (room_name) + 34, 32);
            set_keyiv_in_room(room_name, keyiv);
            break;
        case ASK_JOIN_ROOM_SEC:
            printf ("Request from %s to join room %s\n", mess.sender, mess.content);
            break;
        case QUIT_ROOM_SEC:
            printf("-------- DEBUT QUIT_ROOM_SEC clientsec-cli -------\n");
            printf("The user %s has been deleted from %s \n",mess.sender, mess.content);
            strcpy(text, "/QUIT_ROOM ");
            strcat(text, strtok(mess.content, "|"));
            send_message(text, NULL);
            break;
        case JOIN_ROOM_SEC:
            printf("Reception join room sec \n");
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name =strdup(strtok(mess.content, "|"));
            memcpy (keyiv->key, mess.content + strlen (room_name) + 1, 32);
            memcpy (keyiv->iv, mess.content + strlen (room_name) + 34, 32);
            add_room(room_name,NULL);
            set_keyiv_in_room(room_name, keyiv);
            printf("Création room sécurisé réussie : name = %s, key = %s, iv = %s\n", room_name, keyiv->key, keyiv->iv);
            strcpy(text, "/JOIN_ROOM ");
            strcat(text,room_name);
            send_message(text, NULL);

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


        case CREATE_ROOM_SEC:
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name = strdup(strtok(mess.content, "|"));
            memcpy (keyiv->key, mess.content + strlen (room_name) + 1, 32);
            memcpy (keyiv->iv, mess.content + strlen (room_name) + 34, 32);;
            set_keyiv_in_room(room_name, keyiv);
            printf("Création room sécurisé réussie : name = %s, key = %s, iv = %s\n", room_name, keyiv->key, keyiv->iv);
            /* add_room(mess.content,NULL); */
            break;
        case CREATE_ROOM_SEC_KO:
	    printf ("Error: Failed to created secured room %s\n", mess.content);
	    strcpy (text, "/DELETE_ROOM ");
	    strcat (text, mess.content);
	    send_message (text, NULL);
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
            if (remove_room(mess.content) == -1) {
                printf("erreur remove_room clientsec-cli\n");
            }
            strcpy(text, "/DELETE_ROOM_SEC ");
            strcat(text, mess.content);
            send_message_sec (text, NULL);
            break;

        case QUIT_ROOM:
            printf("---------- DEBUT QUIT_ROOM clientsec-cli -------\n");
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name =strdup(strtok(mess.content, "|"));
            memcpy (keyiv->key, mess.content + strlen (room_name) + 1, 32);
            memcpy (keyiv->iv, mess.content + strlen (room_name) + 34, 32);
            set_keyiv_in_room(room_name, keyiv);
            printf("The user %s has been deleted from %s \n",mess.sender, room_name);
            //strcpy(text, "/QUIT_ROOM ");
            //strcat(text, mess.content);
            //send_message(text, NULL);
            break;

        case MP_SEC:
            leng = strlen(mess.content) + 1;
            keyiv = get_keyiv_in_room(mess.receiver);
            ciphermess = aes_encrypt(keyiv->key, keyiv->iv, (char *)mess.content, &leng);
            strcpy(text, "/MP ");
            strcat(text, mess.receiver);
            strcat(text," ");
            strcat(text,ciphermess);
            send_message(text, NULL);
            break;

        case MP_SEC_OK:
            keyiv = malloc(sizeof (struct KEY_IV));
            room_name = strdup(strtok(mess.content, "|"));
            add_room(room_name,NULL);
            memcpy (keyiv->key, mess.content + strlen (room_name) + 1, 32);
            memcpy (keyiv->iv, mess.content + strlen (room_name) + 34, 32);
            set_keyiv_in_room(room_name, keyiv);
            break;

        case CREATE_ROOM_KO:
        case QUIT_ROOM_KO:
        case CONNECT_SEC_KO:
        case DELETE_ROOM_KO:
        case MESSAGE_KO:
        case MP_SEC_KO:
            printf("The user %s doesn't exist or is not connected as a secure user",mess.receiver);
        case CONNECT_KO:
            printf ("Error: %s\n", mess.content);
            break;
        case JOIN_ROOM_SEC_KO:
            printf ("The admin of the room %s has rejected your request\n", mess.content);
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
    char *plainmess;
    key_iv keyiv;
    char text[MAX_MESS_SIZE] = " ";


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
        printf("clientsec-cli- mess.code: <%d>\n", mess.code);
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
                //keyiv = malloc(sizeof (struct KEY_IV));
                keyiv = get_keyiv_in_room(mess.receiver);
                lenght = MAX_CIPHERED_SIZE;
                plainmess = aes_decrypt(keyiv->key, keyiv->iv, (char *)mess.content, &lenght);
                printf("[%s @ %s] %s\n", mess.sender, mess.receiver, plainmess);
                //free(keyiv);
                //free(plainmess);
            }
            break;
        case CONNECT:
            printf("--------------------------DEBUT CONNECT clientsec-cli ------------------------\n");
            printf("Debut CONNECT clientsec-cli\n");
            strcpy(text, "/CONNECT_OK");
            send_message_sec(text, NULL);
            break;
        case MP:
            if ((get_keyiv_in_room(mess.receiver) == NULL) && (get_keyiv_in_room(mess.sender) == NULL)) {
                printf ("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
            }
            else {
                //keyiv = malloc(sizeof (struct KEY_IV));
                if (get_keyiv_in_room(mess.receiver)!=NULL){
                    keyiv = get_keyiv_in_room(mess.receiver);
                }
                else {
                    keyiv = get_keyiv_in_room(mess.sender);
                }
                lenght = MAX_CIPHERED_SIZE;
                plainmess = aes_decrypt(keyiv->key, keyiv->iv, (char *)mess.content, &lenght);
                printf("[%s @ %s] %s\n", mess.sender, mess.receiver, plainmess);
                //free(keyiv);
                //free(plainmess);
            }
            break;
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
            printf ("The room %s has been created\n", mess.content);
            break;
        case CONNECT_KO:
            printf("DEBUT CONNECT_KO clientsec-cli\n");
            strcpy(text, "/CONNECT_KO_SEC_OK");
            send_message_sec(text, NULL);
            printf ("Error: %s\n", mess.content);
            break;
        case QUIT_ROOM_KO:
        case DELETE_ROOM_KO:
        case MESSAGE_KO:
        case MP_KO:
            printf ("Error: %s\n", mess.content);
            break;
        case CREATE_ROOM_KO:
            printf ("Error: the room %s is already in use\n", mess.content);
            if (is_room_used (mess.content)) {
                strcpy(text, "/DELETE_ROOM_SEC ");
                strcat(text, mess.content);
                send_message_sec(text, NULL);
            }

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
    if (argc == 3) {
        set_certif_filename (argv[1]);
        set_private_key_filename (argv[2]);
        connect_with_authentication (CHATADDR, CHATPORT, SECADDR, SECPORT);
    } else if (argc < 5 || argc > 5) {
        fprintf (stderr,
                 "Usage: ./clientsec-cli ip_client port_client ip_server port_server certificate private_key\n or   ./clientsec-cli certificate private_key\n");
        exit (EXIT_FAILURE);
    } else {
        set_certif_filename(argv[5]);
        set_private_key_filename(argv[6]);
        connect_with_authentication (argv[1], argv[2], argv[3], argv[4]);
    }

    start_communication ();
    return 0;
}
