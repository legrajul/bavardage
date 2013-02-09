#include "common.h"
#include "user_manager.h"
#ifndef _ROOM_MANAGER_H
#define _ROOM_MANAGER_H

#define HASH_ROOM_SIZE 1997

/**
 * Structure représentant un salon de discussion
 * name  : nom du salon
 * admin : créateur du salon
 * users : liste des utilisateurs du salon
 */
typedef struct ROOM {
	char name[MAX_ROOM_NAME_SIZE];
	user admin;
	user_map users;
}*room;

/**
 * Structure représentant une liste chaînée de salons
 */
typedef struct ROOM_LIST {
	room current;
	struct ROOM_LIST *next;
}*room_list;

/**
 * Structure représentant une table de hachage de salons
 */
typedef room_list *room_map;

/**
 * Fonction qui initialise la table des salons
 * @return -1 si cette table existe déjà, 0 sinon
 */
int init_rooms();

/**
 * Fonction qui calcule le haché d'un nom de salon 
 * ne contenant que des caractères alphanumériques 
 * transformés en entiers comme suit :
 *   '0'..'9' -> 0..9
 *   'A'..'Z' -> 10..35
 *   'a'..'z' -> 42..67
 * Puis le haché est calculé comme suit :
 * pour tout caractère c du nom faire
 *   haché = (haché * 100 + entier correspondant à c) mod HASH_ROOM_SIZE
 * fin pour
 *
 * @param room_name le nom du salon à hacher
 * @return -1 si erreur, le haché du nom du salon sinon
 */
int hash_room_name(char *room_name);

/**
 * Vérifie si le nom de salon est disponible
 * @param room_name le nom du salon
 * @return -1 s'il y a une erreur, 0 si le nom est libre, 1 sinon
 */
int is_room_used(char *room_name);

/**
 * Ajoute un salon
 * @pre !is_room_used (room_name)
 * @post is_room_used (room_name)
 * @param room_name le nom du salon
 * @param admin le créateur du salon
 * @return -1 si erreur, 0 sinon
 */
int add_room(char *room_name, user admin);

/**
 * Supprime un salon
 * @pre is_room_used (room_name)
 * @post !is_room_used (room_name)
 * @param room_name le nom du salon à supprimer
 * @return -1 si erreur, 0 sinon
 */
int remove_room(char *room_name);

/**
 * Récupère l'administrateur du salon
 * @pre is_room_used (room_name)
 * @param room_name le nom du salon
 * @return NULL si erreur, l'administrateur du salon sinon
 */
user get_admin(char *room_name);

/**
 * Ajoute un utilisateur dans un salon
 * @param u utilisateur
 * @param room_nam nom du salon
 * @return -1 si erreur, 0 sinon
 */
int add_user_in_room(user u, char *room_name);

/**
 * Retire un utilisateur d'un salon
 * @param u utilisateur
 * @param room_name nom du salon
 * @return -1 si erreur, 0 sinon
 */
int remove_user_from_room(user u, char *room_name);

/**
 * Récupère la liste des utilisateurs d'un salon
 * @param room_name nom du salon
 * @return la liste des utilsateurs du salon
 */
user_list get_users(char *room_name);

int is_user_in_room(user u, char *room_name);

#endif
