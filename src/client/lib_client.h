#ifndef LIB_CLIENT
#define LIB_CLIENT

int extract_code (const char *str);

/**
*	Découpe une chaine de caractère
*	@param s chaine de caractère
*	@param start position de départ dans la chaine s
*	@param end position de fin dans la chaine s
*	@return la chaine s de la position start jusqu'a end
*/
char *str_sub (const char *s, unsigned int start, unsigned int end);

/**
*	Envoi un message au serveur de la forme "message" ou
*	fait appel a send_command si message de la forme /COMMAND "message"
*	@param mess message envoyé au serveur
*	@return 0 ou -1 en cas d'erreur
*/
int send_message (const char *mess);

/** creez un tableau a partir d'une chaine 
 *  @param une chaine de caractere
 *  @return un tableau de chaine de caractere
 */
char **create_table_param(const char *string);

/** retourne la longueur d'un tableau
 *  @param un tableau de pointeur
 *  @return la longueur du tableau
 */
int len (char **tab);

/**
*	Envoi une commande au serveur
*	@param code numéro de la commande
*	@param param paramètre de la commande
*	@return 0 ou -1 en cas d'erreur
*/
int send_command ();

/**
*	Ce connecte au serveur
*	@param addr adresse IP du serveur
*	@param port port du serveur
*	@return 0 ou -1 en cas d'erreur
*/
int connect_socket (const char *addr, const int port);

/**
*	Ce charge de l'envoi des requetes au serveur
*/
void *traitement_send(void *param);

/**
*	Ce charge de la réception des réponses du serveur
*/
void *traitement_recv(void *param);

int disconnect ();

/**
*	Débute la communication avec le serveur en lancant les threads
*	pour envoyer de requetes et lire les réponses du serveur 
*/
int start_communication ();

int switch_to_room (const char *room_name);

int ask_to_join_room (const char *room_name);

/* struct rooms
	char** rooms
	char* current_room
	char* login 
*/

#endif
