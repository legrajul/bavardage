#include <string.h>

#include "room_manager.h"
#include "user_manager.h"

room_map rooms;

int init_rooms () {
  rooms = (room_map) malloc (HASH_ROOM_SIZE * sizeof (room_list));
  // Créer le salon d'accueil
  if (rooms == NULL) {
    return -1;
  } else {
    return 0;
  }
}


int hash_room_name (char *room_name) {
  if (room_name == NULL) {
    return -1;
  }
  int hash = 0;
  int i;
  for (i = 0; room_name[i] != '\0'; i++) {
    char c = room_name[i];
    if ('0' <= c && c <= '9') {
      hash = (hash * 100 + (int) (c - '0')) % HASH_ROOM_SIZE;
    } else {
      hash = (hash * 100 + (int) (c - 'A' + 10)) % HASH_ROOM_SIZE;
    }
  }
  return hash;
}

int is_room_used (char *room_name) {
  if (room_name == NULL) {
    return -1;
  }
  
  // On récupère la liste correspondant au haché du nom de salon
  int index = hash_room_name (room_name);
  room_list l = rooms[index];
  room_list t;

  // On parcourt cette liste
  for (t = l; t != NULL; t = t->next) {
    if (strcmp (t->current->name, room_name) == 0) {
      return 1;
    }
  }

  return 0;
}

int add_room (char *room_name, user *admin) {
  if (is_used_room (room_name)) {
    return -1;
  }
  
  // Création d'un salon et initialisation des champs
  room *r;
  r = (room *) malloc (sizeof (strut ROOM));
  if (strcpy (r->name, room_name) == NULL) {
    return -1;
  }
  if (memcpy (r->admin, admin, sizeof (struct USER)) == NULL) {
    return -1;
  }
  r->users = create_user_map ();
  add_user (admin->name, &(r->users));

  // Ajout du salon dans la map
  int index = hash_room_name (room_name);
  room_list l = rooms[index];
  room_list t;
  t = (room_list) malloc (sizeof (struct ROOM_LIST));
  t->current = r;
  t->next = l;
  rooms[index] = t;
  
  return 0;
}

int remove_room (char *room_name) {
  if (!is_used_room (room_name)) {
    return -1;
  }

  // On récupère la liste des salons correspondant au haché du nom de salon
  int index = hash_room_name (room_name);
  room_list l = rooms[index];
  room_list t, prec = NULL;

  // On parcourt la liste avec un pointeur sur l'élément précédent
  for (t = l; t != NULL; t = t->next) {
    if (strcmp (t->current->name, room_name) == 0) {
      prec->next = t->next;
      t->next = NULL;
      free (t->current);
      free (t);
      return 0;
    }
    prec = t;
  }

  return -1;
}

user *get_admin (char *room_name) {
  if (!is_room_used (room_name)) {
    return NULL;
  }
  
  // On récupère la liste des salons du haché du nom de salon
  int index = hash_room_name (room_name);
  room_list l = rooms[index];
  room_list t;

  // On parcourt la liste
  for (t = l; t != NULL; t = t->next) {
    if (strcmp (t->current->name, room_name) == 0) {
      return t->current->admin;
    }
  }

  return NULL;
}
