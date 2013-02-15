#ifndef LIB_CLIENT
#define LIB_CLIENT

#include "../common/common.h"

int extract_code(const char *str);

/**
 *	Découpe une chaine de caractère
 *	@param s chaine de caractère
 *	@param start position de départ dans la chaine s
 *	@param end position de fin dans la chaine s
 *	@return la chaine s de la position start jusqu'a end
 */
char *str_sub(const char *s, unsigned int start, unsigned int end);

/**
 *	Envoi un message au serveur de la forme "message" ou
 *	fait appel a send_command si message de la forme /COMMAND "message"
 *	@param mess message envoyé au serveur
 *	@return 0 ou -1 en cas d'erreur
 */
int send_message(const char *mess, char **error_mess);

/** creez un tableau a partir d'une chaine 
 *  @param une chaine de caractere
 *  @return un tableau de chaine de caractere
 */
char **create_table_param(const char *string);

/** retourne la longueur d'un tableau
 *  @param un tableau de pointeur
 *  @return la longueur du tableau
 */
int len(char **tab);

/**
 *	Envoi une commande au serveur
 *	@param code numéro de la commande
 *	@param param paramètre de la commande
 *	@return 0 ou -1 en cas d'erreur
 */
int send_command();

/**
 *	Ce connecte au serveur
 *	@param addr adresse IP du serveur
 *	@param port port du serveur
 *	@return 0 ou -1 en cas d'erreur
 */
int connect_socket(const char *addr, const int port);

int disconnect();

int get_last_request_code();

int is_connected();

int receive_message(message *m);

char *get_login ();

#endif
