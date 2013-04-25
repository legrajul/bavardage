#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user_manager.h"

int hash_user (char *login) {
	if (login == NULL)
		return -1;
	int hash = 0;
	int i;
	for (i = 0; login[i] != '\0'; i++) {
		char c = login[i];
		if (c >= '0' && c <= '9') {
			hash = (hash * 100 + (int) (c - '0')) % HASH_USER_MAP_SIZE;
		} else {
			hash = (hash * 100 + (int) (c - 'A' + 10)) % HASH_USER_MAP_SIZE;
		}
	}
	return hash;
}

user_map create_user_map () {
	user_map map = (user_map) malloc (HASH_USER_MAP_SIZE * sizeof(user_list));
	if (map == NULL) {
		return NULL;
	} else {
		return map;
	}
}

int is_login_used (char *login, user_map map) {
	if (login == NULL)
		return -1;

	int index = hash_user (login);
	user_list l = map[index];
	user_list t;

	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current_user->name, login) == 0)
			return 1;
	}

	return 0;
}

int add_user (user u, user_map map) {
	if (is_login_used (u->name, map))
		return -1;

	// Ajout du user dans la liste
	int index = hash_user (u->name);
	user_list l = map[index];
	user_list t;
	t = (user_list) malloc (sizeof(struct USER_LIST));
	t->current_user = u;
	t->next = l;
	map[index] = t;

	return 0;
}

int remove_user (user u, user_map map) {
	if (!is_login_used (u->name, map))
		return -1;

	int index = hash_user (u->name);
	user_list l = map[index];
	user_list t, prec = NULL;

	int count = 0;
	for (t = l; t != NULL; t = t->next) {
		count++;
		if (strcmp (t->current_user->name, u->name) == 0) {
			if (prec != NULL)
				prec->next = t->next;
			t->next = NULL;
		}
		free (t);
		prec = t;
	}
	if (count == 1) {
		map[index] = NULL;
	}

	return -1;
}

user get_user (char *login, user_map map) {
	int index = hash_user (login);
	user_list l = map[index];
	user_list t;

	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current_user->name, login) == 0) {
			return t->current_user;
		}
	}
	return t->current_user;
}

