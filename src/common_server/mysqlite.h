#ifndef _MYSQLITE_H
#define _MYSQLITE_H

#define QUERY_SIZE 524

// commun au serveurs

/**
 *   Connecte le server Ã  la base de donnÃ©es
 *   @return 1 si la connexion s'est bien effectuÃ©e, -1 sinon
 */
int connect_server_database(const char *fileDb);

/**
 *   Ajoute un utilisateur dans la base de donnÃ©es
 *	 @param login 
 *   @return 1 si la l'ajout s'est bien effectuÃ©, -1 sinon
 */
int add_user_db(char *login);

/**
 *   Supprime un utilisateur dans la base de donnÃ©es
 *	 @param login 
 *   @return 1 si la suppression s'est bien effectuÃ©e, -1 sinon
 */
int delete_user_db(char *login);

/**
 *   VÃ©rifie si un utilisateur est dans la base de donnÃ©es
 *	 @param login 
 *   @return 1 si la vÃ©rification s'est bien effectuÃ©e, -1 sinon
 */
int check_user_db(char *login);

/**
 *   DÃ©connecte le serveur de la base de donnÃ©es
 */
int close_server_database();

#endif
