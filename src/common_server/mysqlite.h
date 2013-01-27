#ifndef _MYSQLITE_H
#define _MYSQLITE_H

  #define QUERY_SIZE 524

  // commun au serveurs
  
/**
 *   Connecte le server à la base de données
 *   @return 1 si la connexion s'est bien effectuée, -1 sinon
 */
  int connect_server_database(const char *fileDb, const char *tableOfDb);
  
/**
 *   Ajoute un utilisateur dans la base de données
 *	 @param login 
 *   @return 1 si la l'ajout s'est bien effectué, -1 sinon
 */
  int add_user(char *login);
  
/**
 *   Supprime un utilisateur dans la base de données
 *	 @param login 
 *   @return 1 si la suppression s'est bien effectuée, -1 sinon
 */
  int delete_user(char *login);
  
/**
 *   Vérifie si un utilisateur est dans la base de données
 *	 @param login 
 *   @return 1 si la vérification s'est bien effectuée, -1 sinon
 */
  int check_user(char *login);
  
/**
 *   Déconnecte le serveur de la base de données
 */  
  void close_server_database();

#endif
