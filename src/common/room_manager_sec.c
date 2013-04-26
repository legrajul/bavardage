#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "room_manager.h"
#include "user_manager.h"
#include "room_manager_sec.h"

room_map rooms;

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
