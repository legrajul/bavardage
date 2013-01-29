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
      hash = (hash * 100 + (int)(c - '0')) % HASH_USER_MAP_SIZE;
    } else {
      hash = (hash * 100 + (int)(c - 'A' + 10)) % HASH_USER_MAP_SIZE; 
    }
  }
  return hash;
}

user_map create_user_map () {
  user_map map = (user_map) malloc(HASH_USER_MAP_SIZE * sizeof(user_list));
  if (map == NULL) {
    return NULL;
  } else {
    return map;
  }
}

int is_login_used (char *login, user_map map) {
  if (login == NULL)
    return -1;

  int index = hash_user(login);
  user_list l = map[index];
  user_list t;

  for (t = l; t != NULL; t = t->next) {
    if (strcmp(t->current_user->name, login) == 0)
      return 1;
  }

  return 0;
}

int add_user (char *login, user_map map) {
  if (is_login_used(login, map))
    return -1;
  // creation du user
  user u;
  u = (user) malloc(sizeof(struct USER));
  strcpy(u->name, login);

  // Ajout du user dans la liste
  int index = hash_user(login);
  user_list l = map[index];
  user_list t;
  t = (user_list) malloc(sizeof(struct USER_LIST));
  t->current_user = u;
  t->next = l;
  map[index] = t;

  return 0;
}

int remove_user (char *login, user_map map) {
  if (!is_login_used(login, map))
    return -1;

  int index = hash_user(login);
  user_list l = map[index];
  user_list t, prec = NULL;

  for (t = l; t != NULL; t = t->next) {
    if (strcmp(t->current_user->name, login) == 0) {
      if (prec != NULL)
	prec->next = t->next;
      t->next = NULL;
      free(t->current_user);
      free(t);
    }
    prec = t;
  }

  return -1;
}

