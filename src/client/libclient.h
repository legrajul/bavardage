#ifndef LIB_CLIENT
#define LIB_CLIENT

#include "../common/common.h"

int extract_code(const char *str);

/**
 *      Découpe une chaine de caractère
 *      @param s chaine de caractère
 *      @param start position de départ dans la chaine s
 *      @param end position de fin dans la chaine s
 *      @return la chaine s de la position start jusqu'a end
 */
char *str_sub(const char *s, unsigned int start, unsigned int end);

/**
 *      Envoi un message au serveur de la forme "message" ou
 *      fait appel a send_command si message de la forme /COMMAND "message"
 *      @param mess message envoyé au serveur
 *      @param error_mess message d'erreur s'il y en a
 *      @return 0 ou -1 en cas d'erreur, -3 en cas d'une mauvaise commande
 */
int send_message(const char *mess, char **error_mess);

/**
 * Créer un tableau a partir d'une chaine
 *  @param une chaine de caractere
 *  @return un tableau de chaine de caractere
 */
char **create_table_param(const char *string);

/**
 * Retourne la longueur d'un tableau
 *  @param un tableau de pointeur
 *  @return la longueur du tableau
 */
int len(char **tab);

/**
 *      Envoi une commande au serveur
 *      @param code numéro de la commande
 *      @param param paramètre de la commande
 *      @return 0 ou -1 en cas d'erreur
 */
int send_command();

/**
 *      Ce connecte au serveur
 *      @param addr adresse IP du serveur
 *      @param port port du serveur
 *      @return 0 ou -1 en cas d'erreur
 */
int connect_socket(const char *addr, const int port);

/**
 * Se déconnecte du serveur
 * @return 0
 */
int disconnect();

/**
 * Renvoie un message envoyé par le serveur
 * @param m le message envoyé
 * @return -1 en cas d'erreur, 0 sinon
 */
int receive_message(message *m);

/**
 * Renvoie le login utilisé
 * @return le login
 */
char *get_login ();

#endif
