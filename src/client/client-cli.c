#include "lib_client.h"
#include "../common/common.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

pthread_t thread_send, thread_recv;

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
	if (receive_message(&mess) < 0) {
	    perror ("readSocketTCP");
	}
	
	if (mess.code == KO) {
	    printf ("Error: %s\n", mess.content);
	    continue;
	}

	int code = get_last_request_code ();
	
	char *res = NULL;
	if (is_connected ()) {
	    if (code == DISCONNECT && mess.code == OK) {
		disconnect ();
		printf ("You're now disconnected from the chat server\n");
		pthread_detach(thread_send);
		exit (0);
	    }
	    switch (mess.code) {
	    case OK:
		if (strlen (mess.content) > 0) {
		    printf ("%s\n", mess.content);
		}
		break;
	    
	
	    case MESSAGE:
		printf ("[%s @ %s] %s\n", mess.sender, mess.receiver, mess.content);
		break;
	    
	    
	    case MP:
		printf ("[%s > %s] %s\n", mess.sender, mess.receiver, mess.content);
		break;

	    case USER_LIST_CHUNK:
		printf("You joined the room %s\n", mess.content);
		res = strdup(mess.content);
		while(mess.code == USER_LIST_CHUNK) {
		    receive_message (&mess);
		    strcat (res, ", ");
		    strcat(res, mess.content);
		}	    
		printf("%s", res);		
		break;

	    case USER_LIST_END:
		printf("The user %s joined the room \n", mess.sender);
		break;
	    
	    case DELETE_ROOM:
		printf ("The room %s has been deleted\n", mess.content);
		break;
	    
	    default:
		break;
	    }
	}
    }
    pthread_exit(0);
}

int start_communication () {
    pthread_create(&thread_recv, NULL, traitement_recv, NULL);
    pthread_create(&thread_send, NULL, traitement_send, NULL);

    pthread_join(thread_recv, NULL);
    pthread_join(thread_send, NULL);

    return 0;
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
	fprintf (stderr, "Usage: ./client ip_server port_server\n");
	exit (EXIT_FAILURE);
    }
    connect_socket (argv[1], atoi(argv[2]));
    start_communication ();
    return 0;
}
