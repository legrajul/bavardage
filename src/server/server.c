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
char *home_room = "accueil";

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

void my_sigaction(int s) {
    switch (s) {
    case SIGINT:
        closeSocketTCP(listen_socket);
        exit(0);
        break;
    default:
        break;
    }
}

int clear_message(message *m) {
    strcpy(m->sender, "");
    strcpy(m->content, "");
    strcpy(m->receiver, "");
    m->code = -1;

}

void *handle_connexion(void *param) {

    SocketTCP *s = (SocketTCP *) param;
    int receive;
    message buffer, response;
    int is_connected = 0;
    user u;
    while (1) {
        clear_message(&buffer);
        clear_message(&response);
        receive = readSocketTCP(s, (char *) &buffer, sizeof(message));
        if (receive > 0) {
            printf("Message received with code %d\n", buffer.code);
            if (buffer.code != CONNECT && u == NULL) {
                strcpy(response.content, "Error");
                response.code = KO;
            } else {
                pthread_mutex_lock(&mutex);
                switch (buffer.code) {
                case CREATE_ROOM:
                    if (is_room_used(buffer.content)) {
                        response.code = KO;
                        strcpy(response.content,
                               "This room name is already in use");
                    } else {
                        add_room(buffer.content, u);
                        add_user_in_room(u, buffer.content);

                        response.code = CREATE_ROOM;
                        strcpy (response.sender, buffer.sender);
                        strcpy (response.content, buffer.content);
                        writeSocketTCP (u->socket, (char *) &response, sizeof (message));

                        response.code = ADD_USER;
                    }
                    break;

                case JOIN_ROOM:
                    if (is_room_used(buffer.content)) {
                        strcpy(response.content, buffer.content);
                        // On envoie la liste des clients connectés à tous les clients connectés au salon
                        user_list users = get_users(buffer.content);
                        user_list t, l;
                        message m;

                        m.code = NEW_USER;
                        strcpy(m.sender, u->name);
                        strcpy(m.content, buffer.content);
                        for (t = users; t != NULL; t = t->next) {
                            writeSocketTCP(t->current_user->socket, (char *) &m,
                                           sizeof(message));
                        }

                        strcpy(response.content, buffer.content);
                        response.code = CREATE_ROOM;
                        writeSocketTCP (u->socket, (char *) &response, sizeof (message));
                        add_user_in_room(u, buffer.content);

                        m.code = ADD_USER;
                        strcpy(m.content, buffer.content);
                        users = get_users(buffer.content);
                        for (l = users; l != NULL; l = l->next) {
                            strcpy(m.sender, l->current_user->name);
                            writeSocketTCP(u->socket, (char *) &m,
                                           sizeof(message));
                        }
                        response.code = OK;
                    } else {
                        strcpy(response.content, "The room does not exist");
                        response.code = KO;
                    }
                    break;

                case QUIT_ROOM:
                    if (u != get_admin(buffer.content)) {
                        remove_user_from_room(u, buffer.content);
                        printf("User successfully deleted");

                        // On envoie la liste des clients connectés à tous les clients connectés au salon
                        user_list users = get_users(buffer.content);
                        user_list t, u;
                        message m;
                        m.code = RM_USER;
                        strcpy(m.sender, buffer.sender);
                        strcpy(m.content, buffer.content);
                        for (t = users; t != NULL; t = t->next) {
                            writeSocketTCP(t->current_user->socket, (char *) &m,
                                           sizeof(message));
                        }
                        response.code = DELETE_ROOM;
                        strcpy (response.content, buffer.content);
                        break;
                    }
                    remove_user_from_room(u, buffer.content);

                case DELETE_ROOM:
                    printf("Room deletion request with name %s by %s\n",
                           buffer.content, buffer.sender);
                    if (!is_room_used(buffer.content)) {
                        response.code = KO;
                        strcpy(response.content, "This room doesn\'t exist");
                    } else {
                        if (u != get_admin(buffer.content)) {
                            response.code = KO;
                            strcpy(response.content,
                                   "You're not admin, you can't delete this room");
                        } else {
                            user_list users = get_users(buffer.content);
                            user_list t;
                            // On demande à tous les clients connectés au salon de le supprimer
                            message m;
                            m.code = DELETE_ROOM;
                            strcpy(m.content, buffer.content);
                            for (t = users; t != NULL; t = t->next) {
                                writeSocketTCP(t->current_user->socket,
                                               (char *) &m, sizeof(message));
                            }
                            response.code = DELETE_ROOM;
                            strcpy (response.content, buffer.content);
                            remove_room(buffer.content);
                        }
                    }
                    break;
		    
                case DISCONNECT:
		    // TODO retirer user des salons où il est connecté
                    printf("Disconnection\n");
                    response.code = OK;
                    if (is_connected) {
                        remove_user(u, server_user_map);
                        remove_user_from_room(u, home_room);
                    }
                    writeSocketTCP(s, (char *) &response, sizeof(message));
                    pthread_mutex_unlock(&mutex);
                    closeSocketTCP(s);
                    pthread_exit(0);
                    break;

                case CONNECT:
                    if (!is_login_valid(buffer.sender)) {
                        response.code = KO;
                        strcpy(response.content, "Login not acceptable");
                    } else if (is_login_used(buffer.sender, server_user_map)
                               == 1) {
                        printf("login already in use : %s\n",
                               buffer.sender);
                        response.code = KO;
                        strcpy(response.content,
                               "Login already in use, change your login");
                    } else {
                        printf("successful connection : %s\n",
                               buffer.sender);
                        strcpy(response.sender, buffer.sender);
                        response.code = CREATE_ROOM;
                        strcpy(response.content, home_room);
                        is_connected = 1;
                        u = (user) malloc(sizeof(struct USER));
                        strcpy(u->name, buffer.sender);
                        u->socket = s;
                        add_user(u, server_user_map);
                        writeSocketTCP (u->socket, (char *) &response, sizeof (message));

                        message m;
                        m.code = NEW_USER;
                        strcpy(m.sender, u->name);
                        strcpy(m.content, home_room);
                        user_list users = get_users (home_room);
                        user_list t, l;
                        for (t = users; t != NULL; t = t->next) {
                            writeSocketTCP(t->current_user->socket, (char *) &m,
                                           sizeof(message));
                        }

                        add_user_in_room(u, home_room);
                        m.code = ADD_USER;
                        strcpy(m.content, home_room);
                        users = get_users(home_room);
                        for (l = users; l != NULL; l = l->next) {
                            strcpy(m.sender, l->current_user->name);
                            writeSocketTCP(u->socket, (char *) &m,
                                           sizeof(message));
                        }
                        response.code = OK;

                    }

                    break;

                case MESSAGE:
                    if (buffer.receiver == NULL) {
                        response.code = KO;
                        strcpy(response.content,
                               "Error this command is not allowed");
                        break;
                    }
                    if (buffer.content == NULL) {
                        response.code = KO;
                        strcpy(response.content,
                               "You can not send an empty message");
                        break;
                    }
                    if (is_room_used(buffer.receiver) != 1) {
                        response.code = KO;
                        strcpy(response.content,
                               "This room does not exist");
                        break;
                    }
                    response.code = OK;
                    user_list l = get_users(buffer.receiver);
                    user_list t;
                    for (t = l; t != NULL; t = t->next) {
                        writeSocketTCP(t->current_user->socket,
                                       (char *) &buffer, sizeof(message));
                    }
                    break;

                case MP:
                    if (!is_login_used(buffer.receiver, server_user_map)) {
                        response.code = KO;
                        strcpy(response.content,
                               "You can not send a message to an non-existing user");
                        break;
                    }
                    if (buffer.content == NULL) {
                        response.code = KO;
                        strcpy(response.content,
                               "You can not send an empty message");
                        break;
                    }

                    user receiver = get_user(buffer.receiver,
                                             server_user_map);
                    writeSocketTCP(receiver->socket, (char *) &buffer,
                                   sizeof(message));

                    response = buffer;
                    break;

                default:
                    break;
                }
                pthread_mutex_unlock(&mutex);
                writeSocketTCP(s, (char *) &response, sizeof(message));
            }
        } else if (receive == -1) {
            printf("Error: exiting thread...\n");
            pthread_exit(0);
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

int create_main_room() {
    printf("Server room created\n");
    init_rooms();
    add_room(home_room, NULL);
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
        printf("New connection\n");
        new_thread(client);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./server ip port\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Now listening to the clients connections...\n");
        start_listening(argv[1], atoi(argv[2]));

    }
    return -1;
}
