#include "lib_server.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include "../common_server/mysqlite.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>
#include <string.h>
#include <sqlite3.h>


SocketTCP *listen_socket;
pthread_mutex_t mutex;

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
        while (1) {
			clear_message (&buffer);
			clear_message (&response);
			receive = readSocketTCP(s, (char *) &buffer, sizeof (message));
			if (receive > 0) {
				pthread_mutex_lock(&mutex);
        		switch (buffer.code) {
        		       	case CREATE_ROOM:
										break;
               
        		       	case QUIT_ROOM:
										break;

						case JOIN_ROOM:
										break;

        		       	case DISCONNECT:																				
										printf("Disconnection\n");
										response.code = OK;
										if (is_connected)
											delete_user (buffer.name);
										writeSocketTCP(s, (char *) &response, sizeof(message));
										pthread_mutex_unlock(&mutex);
										closeSocketTCP(s);                        
										pthread_exit(0);
										break;                        
					                                                             
        		       	case CONNECT:
										if (check_user(buffer.name) == -1) {
											printf ("login already in use : %s\n", buffer.name);
											response.code = LOGIN_IN_USE;
											strcpy (response.mess, "Login already in use, change your login");
									    } else {
											printf ("successful connection : %s\n", buffer.name);
											strcpy(response.name, buffer.name);
											response.code = OK;
											strcpy(response.mess, "Successful connection");
										    is_connected = 1;
										    add_user(buffer.name);
										}
										
										break;

               			case MESSAGE:
										break;

						default:
								break;
           		}
				pthread_mutex_unlock(&mutex);
	            writeSocketTCP(s, (char *) &response, sizeof(message));
	             
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

int start_listening(const char *addr, int port) {
	SocketTCP *client;
	
        
    if ((listen_socket = creerSocketEcouteTCP(addr, port)) == NULL) {
        perror("creerSocketEcouteTCP");
        return -1;
     }

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
		printf ("Setting up the database...\n");		
		connect_server_database("server_database.db");
		printf ("Now listening to the clients connections...\n");
		start_listening(argv[1], atoi(argv[2]));  
		close_server_database(); 

	} 
	return -1;
}
