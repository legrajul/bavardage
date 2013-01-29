#include "lib_client.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

SocketTCP *client_sock;
message *msg;
char *login;
int status = NOT_CONNECTED;
pthread_t thread_send, thread_recv;

void my_sigaction(int s) {
    switch (s) {
        case SIGINT:
			disconnect ();
            break;
        default:
            break;
    }
}

char *str_sub (const char *s, unsigned int start, unsigned int end) {
    char *new_s = NULL;
    if (s != NULL && start < end) {
        new_s = malloc (sizeof (*new_s) * (end - start + 2));
        if (new_s != NULL) {
            int i;
            for (i = start; i < end; i++) {
                new_s[i-start] = s[i];
            }
            new_s[i-start] = '\0';
        } else {
            fprintf (stderr, "Out of memory\n");
            exit (EXIT_FAILURE);
        }
    }
    return new_s;
}

int extract_code (const char *str) {
    char *command = NULL;
    command = str_sub(str, 1, strlen(str));

    if (strcmp(command, "CREATE_ROOM") == 0) {
        return CREATE_ROOM;
    } else if (strcmp(command, "DISCONNECT") == 0) {
        return DISCONNECT;
    } else if (strcmp(command, "CONNECT") == 0) {
        return CONNECT;
    } else if (strcmp(command, "QUIT_ROOM") == 0) {
        return QUIT_ROOM;
    } else if (strcmp(command, "JOIN_ROOM") == 0) {
        return JOIN_ROOM;
    } else if (strcmp(command, "MESSAGE") == 0) {
        return MESSAGE;
    }

    return -1;
}

int start_communication () {
	(void) signal(SIGINT, my_sigaction);
    pthread_create(&thread_recv, NULL, traitement_recv, NULL);
    pthread_create(&thread_send, NULL, traitement_send, NULL);

    pthread_join(thread_recv, NULL);
    pthread_join(thread_send, NULL);

    return 0;
}

int connect_socket (const char *addr, const int port) {
    int co;
	if ((client_sock = creerSocketTCP()) == NULL) {
        perror("creerSocketTCP");
        return -1;
    }
    printf ("Please wait for connecting the server...\n");
    if ((co = connectSocketTCP(client_sock, addr, port)) == -1 ) {
    	perror("connectSocketTCP");
        return -1;
    }

    printf("You can now send commands and messages\n");
    start_communication();

    return 0;
}

void *traitement_send(void *param) {
    char mess[MAX_MESS_SIZE] = "";
    while (fgets(mess, MAX_MESS_SIZE, stdin) != NULL) {
        mess[strlen(mess) - 1] = '\0';
        send_message (mess);
    }
    pthread_exit(0);
}

void *traitement_recv(void *param) {
    message mess;
    while (1) {
		
        if (readSocketTCP(client_sock, (char*) &mess, sizeof(message)) > 0) {
			//~ printf ("message recieved, code = %d, mess = %s, asked_code = %d, status = %d\n", mess.code, mess.mess, msg->code, status);
        }

        if (msg->code == CONNECT && mess.code == OK && status == NOT_CONNECTED) {
			printf ("You're now connected to the chat server with the login: %s\n", login);
            status = CONNECTED;
        }

        if (msg->code == CONNECT && mess.code == LOGIN_IN_USE && status == NOT_CONNECTED) {
			printf ("Error : %s\n", mess.mess);
		}

        if (msg->code == DISCONNECT && mess.code == OK) {
			closeSocketTCP(client_sock);
			printf ("You're now disconnected from the chat server\n");
            pthread_detach(thread_send);
            exit (0);
        }
    }
    pthread_exit(0);
}

int send_command (const int code, const char *param) {
	if (msg == NULL) {
		msg = (message*) malloc(sizeof(message));
	}
    message mess;
    mess.code = code;
    if (login != NULL)
		strcpy(mess.name, login);
    if (param != NULL)
        strcpy(mess.mess, param);
    strcpy(mess.room, "");
    msg->code = mess.code;
    writeSocketTCP(client_sock, (char*) &mess, sizeof(message));
    
    return 0;
}

int send_message (const char *mess) {
    int code;
    char buffer[strlen(mess)];
    strcpy(buffer, mess);
    msg = (message*) malloc(sizeof(message));

    if (mess[0] == '/') {
        code = extract_code(strtok(buffer, " "));
        if (code == -1) {
            perror("extract_code");
            return -1;
        }
        
        msg->code = code;

        switch (code) {
            case CONNECT:
                if (status == NOT_CONNECTED) {
                    char *tmp;
                    tmp = strtok (NULL, " ");
                    if (tmp != NULL) {
						login = strdup (tmp);
					}
                    if (login == NULL) {
						printf ("login null\n");
					}
                    strcpy(msg->name, login);
                    send_command (msg->code, msg->name);
                }         
                break;
            case DISCONNECT:
                strcpy(msg->name, login);
                disconnect();
                break;
        }
        
    } else {
        //envoyer un message classique sur le salon general su server
    }

    return 0;
}

int disconnect() {
    send_command(DISCONNECT, "");
    return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf (stderr, "Usage: ./client ip_server port_server\n");
		exit (EXIT_FAILURE);
	}
    connect_socket (argv[1], atoi(argv[2]));
    return 0;
}

