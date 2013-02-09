#ifndef _USER_MANAGER_H
#define _USER_MANAGER_H
#include "common.h"
#include "SocketTCP.h"

#define HASH_USER_MAP_SIZE 1997

typedef struct USER {
	char name[MAX_NAME_SIZE];
	SocketTCP *socket;
}*user;

typedef struct USER_LIST {
	user current_user;
	struct USER_LIST *next;
}*user_list;

typedef user_list *user_map;

int hash_user(char *login);

user_map create_user_map();

int is_login_used(char *login, user_map map);

int add_user(user u, user_map map);

int remove_user(user u, user_map map);

user get_user(char *login, user_map map);

#endif
