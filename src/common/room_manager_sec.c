#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "room_manager.h"
#include "user_manager.h"
#include "room_manager_sec.h"

extern room_map rooms;

int set_keyiv_in_room (char *room_name, key_iv keyiv) {
	/* printf ("---- BEGIN is_room_used ----\n"); */
	if (room_name == NULL || keyiv == NULL) {
		return -1;
	}

	// On récupère la liste correspondant au haché du nom de salon
	int index = hash_room_name (room_name);
	room_list l = rooms[index];
	room_list t;

	// On parcourt cette liste
	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current->name, room_name) == 0) {
			if(t->current->keyiv == NULL)
			t->current->keyiv = malloc(sizeof(struct KEY_IV));
			 
			memcpy(t->current->keyiv, keyiv, sizeof(struct KEY_IV));
			return 1;
		}
	}
	/* printf ("---- END is_room_used ----\n"); */
	return 0;
}

key_iv get_keyiv_in_room (char *room_name) {
	//printf ("---- BEGIN get_keyiv_in_room ----\n"); 
	if (room_name == NULL) {
		return NULL;
	}
	//printf ("---- get_keyiv_in_room 1----\n"); 
	// On récupère la liste correspondant au haché du nom de salon
	int index = hash_room_name (room_name);
	room_list l = rooms[index];
	room_list t;
	//printf ("---- get_keyiv_in_room 2----\n"); 
	// On parcourt cette liste
	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current->name, room_name) == 0) {			
			return t->current->keyiv;
		}
	}
	//printf ("---- END get_keyiv_in_room ----\n"); 
	return NULL;
}



int add_room_client (char *room_name) {
	/* printf ("---- BEGIN add_room ----\n"); */

	if (is_room_used (room_name)) {
		return -1;
	}

	// Création d'un salon et initialisation des champs
	room r;
	r = (room) malloc (sizeof(struct ROOM));
	if (strcpy (r->name, room_name) == NULL) {
		return -1;
	}

	// Ajout du salon dans la map
	int index = hash_room_name (room_name);
	room_list l = rooms[index];
	room_list t;

	t = (room_list) malloc (sizeof(struct ROOM_LIST));
	t->current = r;
	t->next = l;
	rooms[index] = t;

	/* printf ("---- END add_room ----\n"); */
	return 0;
}
