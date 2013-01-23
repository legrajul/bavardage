#include "lib_client.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

SocketTCP *client_sock;
message *msg;
char *login;
pthread_t thread_send, thread_recv;

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
            fprintf (stderr, "Memoire insuffisante\n");
            exit (EXIT_FAILURE);
        }
    }
    return new_s;
}

int extract_code (const char *str) {
    char *command = NULL;
    command = str_sub(str, 1, strlen(str));

    printf("command: <%s>\n", command);

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
    if ((co = connectSocketTCP(client_sock, addr, port)) == -1 ) {
    	perror("connectSocketTCP");
        return -1;
    }

    printf("Debut de communication\n");
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
        int r = readSocketTCP(client_sock, (char*) &mess, sizeof(message));

        if (r >0) {
            printf("msg.code: %i\n", mess.code);
            printf("msg.mess: %s\n", mess.mess);
        }

        if (msg->code == DISCONNECT && mess.code == OK) {
            pthread_detach(thread_send);
            pthread_exit(0);
            closeSocketTCP(client_sock);
        }
    }
    pthread_exit(0);
}

int send_command (const int code, const char *param) {
    message msg;
    msg.code = code;
    if (param != NULL)
        strcpy(msg.name, param);
    strcpy(msg.mess, "");
    strcpy(msg.room, "");

    printf("msg.code: %i\n", msg.code);
    writeSocketTCP(client_sock, (char*) &msg, sizeof(message));

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
                login = strtok(NULL, "");
                strcpy(msg->name, login);
                send_command (msg->code, msg->name);
                break;
            case DISCONNECT:
                send_command (code, NULL);
                break;
        }
        
    } else {
        //envoyer un message classique sur le salon general su server
    }

    printf("Code: <%i>\n", msg->code);
    printf("Login: <%s>\n", msg->name);

    return 0;
}

/*int main(int argc, char *argv[]) {
    connect_socket (argv[1], atoi(argv[2]));

    return 0;
}*/