#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "room_manager.h"
#include "user_manager.h"
#include "room_manager_sec.h"

extern room_map rooms;

int set_keys_in_room (char *room_name, keys k) {
	
	if (room_name == NULL || k == NULL) {
		return -1;
	}

	// On récupère la liste correspondant au haché du nom de salon
	int index = hash_room_name (room_name);
	room_list l = rooms[index];
	room_list t;

	// On parcourt cette liste
	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current->name, room_name) == 0) {
			if(t->current->k == NULL)
			    t->current->k = malloc(sizeof(struct KEYS));
			
			memcpy(t->current->k, k, sizeof(struct KEYS));
			return 1;
		}
	}
	
	return 0;
}

keys get_keys_from_room (char *room_name) {

	if (room_name == NULL) {
		return NULL;
	}

	// On récupère la liste correspondant au haché du nom de salon
	int index = hash_room_name (room_name);
	room_list l = rooms[index];
	room_list t;
	
	// On parcourt cette liste
	for (t = l; t != NULL; t = t->next) {
		if (strcmp (t->current->name, room_name) == 0) {			
			return t->current->k;
		}
	}

	return NULL;
}
