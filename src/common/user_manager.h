#ifndef _USER_MANAGER_H
#define _USER_MANAGER_H
#include "common.h"
#include "SocketTCP.h"
#include <openssl/ssl.h>
#define HASH_USER_MAP_SIZE 1997

/**
 * Structure représentant un utilisateur
 */
typedef struct USER {
	char name[MAX_NAME_SIZE];
	SocketTCP *socket;
	SSL *ssl;
}*user;

/**
 * Structure de liste chaînée d'utilisateurs
 */
typedef struct USER_LIST {
	user current_user;
	struct USER_LIST *next;
}*user_list;

/**
 * Type représentant une table de hachage d'utilisateurs
 */
typedef user_list *user_map;

/**
 * Calcule le hash d'un login
 * @param login le login
 * @return le hash du login
 */
int hash_user (char *login);

/**
 * Créer une table de hachage d'utilisateurs
 * @return la table de hachage
 */
user_map create_user_map ();

/**
 * Vérifie sur un login est présent dans une table de hachage
 * @param login le login
 * @param map la table de hachage
 * @return 1 si le login est présent, 0 sinon
 */
int is_login_used (char *login, user_map map);

/**
 * Ajoute un utilisateur dans une table de hachage
 * @param u l'utilisateur
 * @param map la table de hachage
 * @return 0 si tout se passe bien, -1 si erreur
 */
int add_user (user u, user_map map);

/**
 * Retire un utilisateur d'une table de hachage
 * @param u l'utilisateur
 * @param map la table de hachage
 * @return 0 si tout se passe bien, -1 si erreur
 */
int remove_user (user u, user_map map);

/**
 * Récupère un utilisateur par son login d'une table de hachage
 * @param login le login à chercher
 * @param map la table de hachage
 * @return l'utilisateur si présent, NULL sinon
 */
user get_user (char *login, user_map map);

#endif
