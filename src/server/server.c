#include "lib_server.h"
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
char *home_room = "accueil";

int is_login_valid (char *login) {
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

int clear_message (message *m) {
    strcpy (m->sender, "");
    strcpy (m->content, "");
    strcpy (m->receiver, "");
    m->code = -1;
    return 0;
}

int join_room (user u, char *room_name) {
    user_list users = get_users (room_name);
    user_list t, l;
    message m;

    m.code = NEW_USER;
    strcpy (m.sender, u->name);
    strcpy (m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        writeSocketTCP (t->current_user->socket, (char *) &m, sizeof(message));
    }

    strcpy (m.content, room_name);
    m.code = CREATE_ROOM;
    writeSocketTCP (u->socket, (char *) &m, sizeof(message));
    add_user_in_room (u, room_name);

    if (get_admin (room_name) != NULL) {
        m.code = ADMIN;
        strcpy (m.content, room_name);
        strcpy (m.sender, get_admin (room_name)->name);
        writeSocketTCP (u->socket, (char *) &m, sizeof(message));
    }

    m.code = ADD_USER;
    strcpy (m.content, room_name);
    users = get_users (room_name);
    for (l = users; l != NULL; l = l->next) {
        if (l->current_user != get_admin (room_name)) {
            strcpy (m.sender, l->current_user->name);
            writeSocketTCP (u->socket, (char *) &m, sizeof(message));
        }
    }
    return 0;
}

int quit_room (user u, char *room_name) {
    user_list users = get_users (room_name);
    user_list t;
    message m;
    m.code = RM_USER;
    strcpy (m.sender, u->name);
    strcpy (m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        writeSocketTCP (t->current_user->socket, (char *) &m, sizeof(message));
    }
    return 0;
}

int delete_room (char *room_name) {
    user_list users = get_users (room_name);
    user_list t;
    // On demande à tous les clients connectés au salon de le supprimer
    message m;
    m.code = DELETE_ROOM;
    strcpy (m.content, room_name);
    for (t = users; t != NULL; t = t->next) {
        writeSocketTCP (t->current_user->socket, (char *) &m, sizeof(message));
    }
    remove_room (room_name);
    return 0;
}

void *handle_connexion (void *param) {
    SocketTCP *s = (SocketTCP *) param;
    int receive;
    message buffer, response;
    user u;
    while (1) {
        clear_message (&buffer);
        clear_message (&response);
        receive = readSocketTCP (s, (char *) &buffer, sizeof(message));
        if (receive > 0) {
            if (buffer.code != CONNECT && buffer.code != DISCONNECT
                && u == NULL) {
                strcpy (response.content, "Error");
                response.code = KO;
            } else {
                pthread_mutex_lock (&mutex);
                switch (buffer.code) {
                case CREATE_ROOM:
                    printf ("debut Create room : %s\n", buffer.content);
                    if (is_room_used (buffer.content)) {
                        response.code = CREATE_ROOM_KO;
                        strcpy (response.content,
                                buffer.content);
                        printf ("Room already in user server\n");
                    } else {
                        add_room (buffer.content, u);
                        add_user_in_room (u, buffer.content);

                        response.code = CREATE_ROOM;
                        strcpy (response.sender, buffer.sender);
                        strcpy (response.content, buffer.content);
                        printf("Avant response au client securisé\n");
                        writeSocketTCP (u->socket, (char *) &response,
                                        sizeof(message));
                        printf("Après response au client securisé\n");

                        response.code = ADMIN;
                        strcpy (response.content, buffer.content);
                        strcpy (response.sender, u->name);
                        writeSocketTCP (u->socket, (char *) &response,
                                        sizeof(message));

                        response.code = OK;
                    }
                    break;

                case JOIN_ROOM:
                    printf ("Join room : %s\n", buffer.content);
                    if (!is_room_used (buffer.content)) {
                        strcpy (response.content, "The room does not exist");
                        response.code = JOIN_ROOM_KO;
                    } else if (is_user_in_room (u, buffer.content)) {
                        strcpy (response.content,
                                "You're already in this room");
                        response.code = JOIN_ROOM_KO;
                    } else {
                        join_room (u, buffer.content);
                        strcpy (response.content, buffer.content);
                        response.code = OK;
                    }
                    break;

                case QUIT_ROOM:
                    if (!is_room_used (buffer.content)) {
                        response.code = QUIT_ROOM_KO;
                        strcpy (response.content, "This room does not exist");
                        break;
                    } else if (strcmp (home_room, buffer.content) == 0) {
                        response.code = QUIT_ROOM_KO;
                        strcpy (response.content,
                                "You cannot leave the home room");
                        break;
                    } else if (u != get_admin (buffer.content)) {
                        quit_room (u, buffer.content);
                        remove_user_from_room (u, buffer.content);
                        printf ("User successfully deleted\n");
                        response.code = DELETE_ROOM;
                        strcpy (response.content, buffer.content);
                        break;
                    } else if (!is_user_in_room (u, buffer.content)) {
                        response.code = QUIT_ROOM_KO;
                        strcpy (response.content, "You are not in this room");
                        break;
                    }

                case DELETE_ROOM:
                    printf ("Room deletion request with name %s by %s\n",
                            buffer.content, buffer.sender);
                    if (!is_room_used (buffer.content)) {
                        response.code = DELETE_ROOM_KO;
                        strcpy (response.content, "This room doesn\'t exist");
                    } else if (u != get_admin (buffer.content)) {
                        response.code = DELETE_ROOM_KO;
                        strcpy (response.content,
                                "You're not admin, you can't delete this room");
                    } else {
                        delete_room (buffer.content);
                        response.code = DELETE_ROOM;
                        strcpy (response.content, buffer.content);
                    }
                    break;

                case DISCONNECT:
                    printf ("Disconnection from server\n");
                    response.code = DISCONNECT;
                    if (u != NULL) {
                        for (room_list l = get_user_rooms (u); l != NULL;
                             l = l->next) {
                            if (u == get_admin (l->current->name)) {
                                delete_room (l->current->name);
                            } else {
                                quit_room (u, l->current->name);
                                remove_user_from_room (u, l->current->name);
                            }
                        }
                        remove_user (u, server_user_map);
                        free (u);//DISCONNECT_SEC

                    }
                    response.code = DISCONNECT;
                    writeSocketTCP (s, (char *) &response, sizeof(message));
                    printf("writeSocketTCP server\n");
                    pthread_mutex_unlock (&mutex);
                    closeSocketTCP (s);
                    pthread_exit (0);
                    break;

                case CONNECT:
                	printf("--------------------------DEBUT CONNECT server ------------------------\n");
                    if (u != NULL) {
                        response.code = CONNECT_KO;
                        strcpy (response.content, "You are already connected");
                    } else if (!is_login_valid (buffer.sender)) {
                        response.code = CONNECT_KO;
                        strcpy (response.content, "Login not acceptable");
                    } else if (is_login_used (buffer.sender, server_user_map)
                               == 1) {
                        printf ("login already in use : %s\n", buffer.sender);
                        response.code = CONNECT_KO;
                        strcpy (response.content,
                                "Login already in use, change your login");
                    } else {
                        printf ("successful connection : %s\n", buffer.sender);
                        response.code = CONNECT;
                        writeSocketTCP (s, (char *) &response, sizeof(message));

                        u = (user) malloc (sizeof(struct USER));
                        strcpy (u->name, buffer.sender);
                        u->socket = s;
                        add_user (u, server_user_map);
                        join_room (u, home_room);
                        response.code = OK;

                    }

                    break;

                case MESSAGE:
                    if (buffer.receiver == NULL) {
                        response.code = MESSAGE_KO;
                        strcpy (response.content,
                                "Error this command is not allowed");
                        break;
                    }
                    if (buffer.content == NULL) {
                        response.code = MESSAGE_KO;
                        strcpy (response.content,
                                "You can not send an empty message");
                        break;
                    }
                    if (is_room_used (buffer.receiver) != 1) {
                        response.code = MESSAGE_KO;
                        strcpy (response.content, "This room does not exist");
                        break;
                    }
                    user sender = (user) malloc (sizeof(struct USER));
                    strcpy (sender->name, buffer.sender);
                    if (is_user_in_room (u, buffer.receiver) != 1) {
                        response.code = MESSAGE_KO;
                        strcpy (response.content,
                                "You are not allowed to send a message to this room");
                        break;
                    }
                    response.code = OK;
                    user_list l = get_users (buffer.receiver);
                    user_list t;
                    for (t = l; t != NULL; t = t->next) {
                        writeSocketTCP (t->current_user->socket,
                                        (char *) &buffer, sizeof(message));
                    }
                    break;

                case MP:
                    printf("debut MP server\n");

					if (!is_login_used (buffer.receiver, server_user_map)) {
						response.code = MP_KO;
						strcpy (response.content,
								"You can not send a message to an non-existing user");
						break;
					}
					if (strcmp(buffer.content, "") == 0) {
						response.code = MP_KO;
						strcpy (response.content,
								"You can not send an empty message");
						break;
					}

                    printf("buffer.receiver: <%s>\n", buffer.receiver);
					user receiver = get_user (buffer.receiver, server_user_map);
					writeSocketTCP (receiver->socket, (char *) &buffer,
							sizeof(message));

					printf("buffer.code: <%d>\n", buffer.code);
					response = buffer;
					break;

				default:
					break;
				}
				pthread_mutex_unlock (&mutex);  
				writeSocketTCP (s, (char *) &response, sizeof(message));
			}
		} else {
			printf ("Error: the connection with the client just stopped\n");
			if (u != NULL) {
				for (room_list l = get_user_rooms (u); l != NULL; l = l->next) {
					if (u == get_admin (l->current->name)) {
						delete_room (l->current->name);
					} else {
						quit_room (u, l->current->name);
						remove_user_from_room (u, l->current->name);

					}
				}
				remove_user (u, server_user_map);
				free (u);
			}
			printf ("Error: exiting thread...\n");
			pthread_exit (0);
		}

	}
	return NULL;

}

void new_thread (SocketTCP *socket) {
    int ret;

    pthread_attr_t attr;
    if ((ret = pthread_attr_init (&attr)) != 0) {
        fprintf (stderr, "pthread_attr_init: %s\n", strerror (ret));
        exit (EXIT_FAILURE);
    }

    // On dÃ©tache le thread afin de ne pas avoir Ã  faire de join
    if ((ret = pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED))
        != 0) {
        fprintf (stderr, "pthread_attr_setdetachstate: %s\n", strerror (ret));
        exit (EXIT_FAILURE);
    }

    pthread_t t;
    if ((ret = pthread_create (&t, NULL, handle_connexion, (void*) socket))
        != 0) {
        fprintf (stderr, "pthead_create: %s\n", strerror (ret));
        exit (EXIT_FAILURE);
    }

    if ((ret = pthread_attr_destroy (&attr)) != 0) {
        fprintf (stderr, "pthread_attr_destroy: %s\n", strerror (ret));
        exit (EXIT_FAILURE);
    }
}

int create_main_room () {
    printf ("Server room created\n");
    init_rooms ();
    add_room (home_room, NULL);
    server_user_map = (user_map) malloc (
                                         HASH_USER_MAP_SIZE * sizeof(user_list));
    return 0;
}

int start_listening (const char *addr, int port) {
    SocketTCP *client;

    if ((listen_socket = creerSocketEcouteTCP (addr, port)) == NULL) {
        perror ("creerSocketEcouteTCP");
        return -1;
    }

    create_main_room ();

    (void) signal (SIGINT, my_sigaction);
    pthread_mutex_init (&mutex, NULL);
    while (1) {
        client = acceptSocketTCP (listen_socket);
        printf ("New connection\n");
        new_thread (client);
    }
    return -1;
}

int main (int argc, char *argv[]) {
    if (argc < 3) {
        fprintf (stderr, "Usage: ./server ip port\n");
        exit (EXIT_FAILURE);
    } else {
        printf ("Now listening to the clients connections...\n");
        start_listening (argv[1], atoi (argv[2]));

    }
    return -1;
}
