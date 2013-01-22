#include "common.h"

typedef struct {
        char[MAX_NAME_SIZE] name;
        SocketTCP s;
} user;

typedef struct  {
        char[MAX_ROOM_NAME_SIZE] login;
        user admin;
        user *users;
} room;

void *handle_connexion(void *param);
int setup_socket();
int create_room(char *room_name);
int set_ip(char *ip);
int set_port(int p);
