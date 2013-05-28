#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "room_manager.h"
#include "user_manager.h"

room_map rooms;

int init_rooms () {
    rooms = (room_map) malloc (HASH_ROOM_SIZE * sizeof(room_list));
    for (int i = 0; i < HASH_ROOM_SIZE; i++) {
        rooms[i] = NULL;
    }
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

int add_room (char *room_name, user admin) {

    if (is_room_used (room_name)) {
        return -1;
    }

    // Création d'un salon et initialisation des champs
    room r;
    r = (room) malloc (sizeof(struct ROOM));
    if (strcpy (r->name, room_name) == NULL) {
        return -1;
    }

    r->admin = admin;
    r->users = create_user_map ();
    r->k = NULL;

    if (admin != NULL)
        add_user (admin, r->users);

    // Ajout du salon dans la map
    int index = hash_room_name (room_name);
    room_list l = rooms[index];
    room_list t;

    t = (room_list) malloc (sizeof(struct ROOM_LIST));
    t->current = r;
    t->next = l;
    rooms[index] = t;

    return 0;
}

int remove_room (char *room_name) {
    if (!is_room_used (room_name)) {
        return -1;
    }

    // On récupère la liste des salons correspondant au haché du nom de salon
    int index = hash_room_name (room_name);
    room_list l = rooms[index];
    room_list t, prec = NULL;

    // On parcourt la liste avec un pointeur sur l'élément précédent
    int count = 0;
    for (t = l; t != NULL; t = t->next) {
        count++;
        if (strcmp (t->current->name, room_name) == 0) {
            if (prec != NULL) {
                prec->next = t->next;
            }
            t->next = NULL;
            free (t->current);
            free (t);
            if (count == 1) {
                rooms[index] = NULL;
            }
            return 0;
        }
        prec = t;
    }

    return -1;
}

user get_admin (char *room_name) {
 
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

int add_user_in_room (user u, char *room_name) {

    // On récupère la liste des salons du haché du nom de salon
    int index = hash_room_name (room_name);
    room_list l = rooms[index];
    room_list t;

    // On parcourt la liste
    for (t = l; t != NULL; t = t->next) {
        if (strcmp (t->current->name, room_name) == 0) {
            add_user (u, t->current->users);
            break;
        }
    }

    return 0;
}

int remove_user_from_room (user u, char *room_name) {
   
    // On récupère la liste des salons du haché du nom de salon
    int index = hash_room_name (room_name);
    room_list l = rooms[index];
    room_list t;

    // On parcourt la liste
    for (t = l; t != NULL; t = t->next) {
        if (strcmp (t->current->name, room_name) == 0) {
            remove_user (u, t->current->users);
            break;
        }
    }

    return 0;
}

user_list get_users (char *room_name) {
  
    // On récupère la liste des salons du haché du nom de salon
    int index = hash_room_name (room_name);
    room_list l = rooms[index];
    room_list t;
    user_list res = NULL;

    user_map map = NULL;
    // On parcourt la liste
    for (t = l; t != NULL; t = t->next) {
        if (strcmp (t->current->name, room_name) == 0) {
            map = t->current->users;
        }
    }

    if (map != NULL) {
        user_list last = NULL;
        user_list tmp = NULL;
        int i;
        for (i = 0; i < HASH_USER_MAP_SIZE; i++) {
            if (map[i] != NULL) {
                user_list list = map[i];
                user_list l;
                for (l = list; l != NULL; l = l->next) {
                    tmp = (user_list) malloc (sizeof(struct USER_LIST));

                    tmp->current_user = l->current_user;
                    if (last == NULL) {
                        res = tmp;
                    } else {
                        last->next = tmp;
                    }
                    tmp->next = NULL;
                    last = tmp;
                }
            }
        }
    }

  
    return res;
}

int is_user_in_room (user u, char *room_name) {
   
    user_list l = get_users (room_name);
    for (; l != NULL; l = l->next) {
        if (strcmp (l->current_user->name, u->name) == 0) {
	    
            return 1;
        }
    }

    return 0;
}

room_list get_user_rooms (user u) {
    room_list res = NULL;
    room_list prec;
    for (int i = 0; i < HASH_ROOM_SIZE; i++) {
        for (room_list l = rooms[i];
             l != NULL && l->current != NULL && l->current->name != NULL; l =
                 l->next) {
            if (is_user_in_room (u, l->current->name)) {
                if (res == NULL) {
                    res = (room_list) malloc (sizeof(struct ROOM_LIST));
                    res->current = l->current;
                    res->next = NULL;
                    prec = res;
                } else {
                    room_list tmp;
                    tmp = (room_list) malloc (sizeof(struct ROOM_LIST));
                    tmp->current = l->current;
                    tmp->next = NULL;
                    prec->next = tmp;
                    prec = tmp;
                }
            }
        }
    }
    return res;
}


