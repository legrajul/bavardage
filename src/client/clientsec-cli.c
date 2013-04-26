#include "libclientsec.h"
#include "libclient.h"
#include "../common/commonsec.h"
#include "../common/common.h"
#include "../common/room_manager.h"

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



pthread_t thread_send, thread_recv;

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
	}
	pthread_exit (0);
}

void *traitement_recv (void *param) {
	message mess;
	key_iv keyiv;
    unsigned char *plainmess;
    EVP_CIPHER_CTX de;
    char room_name;
	while (1) {
		if (receive_message_sec(&mess) == -1) {
            if (receive_message(&mess) < 0) {
                perror("readSocketTCP");
                pthread_detach(thread_recv);
                pthread_detach(thread_send);
                exit(EXIT_FAILURE);
            }
    
			if(mess.code == OK) {
			  strcpy(room_name, strtok(mess.content, "|"));
			  memcpy(keyiv, strtok(NULL, "|"), sizeof(key_iv));
			}

        if (mess.code == KO) {
            printf("Error: %s\n", mess.content);
            continue;
        }

        char *res = NULL;
        if (mess.code == DISCONNECT) {
            disconnect();
            printf("You're now disconnected from the chat server\n");
            pthread_detach(thread_send);
            exit(0);
        }
        switch (mess.code) {
        case OK:
            if (strlen(mess.content) > 0) {
                printf("%s\n", mess.content);
            }
            break;

        case MESSAGE:
            
            printf("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
            break;

        case MP:
            printf("[%s > %s] %s\n", mess.sender, mess.receiver, mess.content);
            break;

        case NEW_USER:
            printf("The user %s joined the room %s\n", mess.sender, mess.content);
            break;
        case ADD_USER:
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
} 

int start_communication () {
	pthread_create (&thread_recv, NULL, traitement_recv, NULL);
	pthread_create (&thread_send, NULL, traitement_send, NULL);

	pthread_join (thread_recv, NULL);
	pthread_join (thread_send, NULL);

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
