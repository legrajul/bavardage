#include "lib_server.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
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

SocketTCP *listen_socket;
pthread_mutex_t mutex;
user_map server_user_map;
char *server_room = "server_room";

int is_login_valid (char *login) {
    int i = 0;
    char c = login[0];
    while (c != '\0') {
	if (!('0' <= c && c <= '9')
	    && !('a' <= c && c <= 'z')
	    && !('A' <= c && c <= 'Z')) {
	    return 0;
	}
	c = login[++i];
    }
    return 1;
}

void my_sigaction(int s) {
    switch (s) {
    case SIGINT:
	closeSocketTCP(listen_socket);
	exit (0);
	break;
    default:
	break;
    }
}

int clear_message (message *m) {
    strcpy (m->name, "");
    strcpy (m->mess, "");
    strcpy (m->room, "");
    m->code = -1;
	
}

void *handle_connexion(void *param) {

    SocketTCP *s = (SocketTCP *) param;
    int receive;
    message buffer;
    message response;
    int is_connected = 0;
    user u;
    while (1) {
	clear_message (&buffer);
	clear_message (&response);
	receive = readSocketTCP(s, (char *) &buffer, sizeof (message));
	if (receive > 0) {
		if(buffer.code != CONNECT && u == NULL) {
			strcpy(response.mess, "Error");
			response.code = KO; 
		} else {
			pthread_mutex_lock(&mutex);
			switch (buffer.code) {
			case CREATE_ROOM:
			printf ("Room creation request with name %s by %s\n", buffer.mess, buffer.name);
			if (is_room_used (buffer.mess)) {
				response.code = KO;
				strcpy (response.mess, "This room name is already in use");
			} else {
				add_room (buffer.mess, u); 
				add_user_in_room (u, buffer.mess);
				response.code = OK;
			}
			break;

			case JOIN_ROOM:				
				if(is_room_used(buffer.mess)) {
					add_user_in_room (u, buffer.mess);
					strcpy(response.mess, "User added successfully to room");
					response.code = OK;
					printf("User added successfully to room\n");
				} else {											
					strcpy(response.mess, "The room does not exist");
					response.code = KO;
					printf("The room does not exist\n");
				}       		       	
				break;  
				
			case QUIT_ROOM:
				if(u != get_admin (buffer.mess)) {
					remove_user_from_room (u, buffer.mess);
					strcpy(response.mess, "User successfully deleted");
					response.code = OK;  
					printf("User successfully deleted");
					break;
				}
				remove_user_from_room (u, buffer.mess);
				
			case DELETE_ROOM:
			// TODO: Tester si le user demandeur est admin
			printf ("Room deletion request with name %s by %s\n", buffer.mess, buffer.name);
			if (!is_room_used (buffer.mess)) {
				response.code = KO;
				strcpy (response.mess, "This room doesn\'t exist");
			} else {
				if (u != get_admin (buffer.mess)) {
				response.code = KO;
				strcpy (response.mess, "You're not admin, you can't delete this room");
				} else {
				user_list users = get_users (buffer.mess);
				user_list t;
				// On demande à tous les clients connectés au salon de le supprimer
				message m;
				m.code = DELETE_ROOM;
				strcpy (m.mess, buffer.mess);
				for (t = users; t != NULL; t = t->next) {
					writeSocketTCP (t->current_user->socket, (char *) &m, sizeof (message));
				}
								
				remove_room (buffer.mess);
				}

				
			case DISCONNECT:	
				//TODO retirer le user de tous les salons où il est connecté
					printf("Disconnection\n");
				response.code = OK;
				if (is_connected) {
				remove_user(u, server_user_map);
				remove_user_from_room(u, server_room);
				}
				writeSocketTCP(s, (char *) &response, sizeof(message));
				pthread_mutex_unlock(&mutex);
				closeSocketTCP(s);                        
				pthread_exit(0);
				break;                        
																					 
			case CONNECT:
				if (!is_login_valid (buffer.name)) {
				response.code = KO;
				strcpy (response.mess, "Login not acceptable");
				} else if (is_login_used(buffer.name, server_user_map) == 1) {
				printf ("login already in use : %s\n", buffer.name);
				response.code = KO;
				strcpy (response.mess, "Login already in use, change your login");
				} else {
				printf ("successful connection : %s\n", buffer.name);
				strcpy(response.name, buffer.name);
				response.code = OK;
				strcpy(response.mess, "Successful connection");
				is_connected = 1;
				u = (user) malloc(sizeof(struct USER));
				strcpy(u->name, buffer.name);
				u->socket = s;
				add_user(u, server_user_map);
				add_user_in_room(u, server_room);
				}

				break;

			case MESSAGE:
				break;

			default:
				break;
			}
			}
			pthread_mutex_unlock(&mutex);
			writeSocketTCP(s, (char *) &response, sizeof(message));
		}
	} else if (receive == -1) {
	    printf ("Error: exiting thread...\n");
	    pthread_exit (0);
	}
    
    } 
    return NULL;

}



void new_thread(SocketTCP *socket) {
    int ret;

    pthread_attr_t attr;
    if ((ret = pthread_attr_init(&attr)) != 0) {
	fprintf(stderr, "pthread_attr_init: %s\n", strerror(ret));
	exit(EXIT_FAILURE);
    }
  
    // On dÃ©tache le thread afin de ne pas avoir Ã  faire de join
    if ((ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
	fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(ret));
	exit(EXIT_FAILURE);
    }

  
    pthread_t t;
    if ((ret = pthread_create(&t, NULL, handle_connexion, (void*) socket)) != 0) {
	fprintf(stderr, "pthead_create: %s\n", strerror(ret));
	exit(EXIT_FAILURE);
    }

    if ((ret = pthread_attr_destroy(&attr)) != 0) {
	fprintf(stderr, "pthread_attr_destroy: %s\n", strerror(ret));
	exit(EXIT_FAILURE);
    }
}

int create_main_room () {
    printf("Server room created\n");
    init_rooms();
    add_room(server_room, NULL);
    server_user_map = (user_map) malloc(HASH_USER_MAP_SIZE * sizeof(user_list));
}

int start_listening(const char *addr, int port) {
    SocketTCP *client;
	
    if ((listen_socket = creerSocketEcouteTCP(addr, port)) == NULL) {
	perror("creerSocketEcouteTCP");
	return -1;
    }

    create_main_room();

    (void) signal(SIGINT, my_sigaction);
    pthread_mutex_init(&mutex, NULL);
    while (1) {
	client = acceptSocketTCP(listen_socket);
	printf ("New connection\n");  
	new_thread (client);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
	fprintf (stderr, "Usage: ./server ip port\n");
	exit (EXIT_FAILURE);
    } else {				
	printf ("Now listening to the clients connections...\n");
	start_listening(argv[1], atoi(argv[2]));  

    } 
    return -1;
}
