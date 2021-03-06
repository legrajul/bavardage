#ifndef LIB_SERVER
#define LIB_SERVER
#include "../common/common.h"
#include "../common/user_manager.h"
#include "../common/SocketTCP.h"

/**
 *   Traite une connexion lancée dans un thread
 *   Lit les données envoyées par le client et
 *   Envoie une réponse au client selon la commande envoyée par le client
 *      @param param Socket du client
 */

void *handle_connexion (void *param);

/**
 *   Crée la socket serveur
 *   Se met en écoute pour attendre la connexion d'un client
 *   Crée un thread
 *      @param addr adresse ip du serveur
 *   @param port port du serveur
 *   @return -1 si erreur de création de socket ou de thread
 */

int start_listening (const char *addr, int port);

/**
 *   Initie un thread
 *   Crée un thread
 *   Détache le thread
 *   Détruit le thread
 *      @param socket socket client
 */

void new_thread (SocketTCP *socket);

/**
 * Vide la structure message
 * @param m structure message
 */
int clear_message (message *m);

/**
 * Créer le salon d'accueil
 * @return -1 si erreur, 0 sinon
 */
int create_main_room ();

/**
 * Créer un salon
 * @param room_name le nom du salon
 * @return -1 si erreur, 0 sinon
 */
int create_room (char *room_name);

/**
 * Ajouter un utilisateur dans un salon
 * @param u l'utilisateur
 * @param room_name le nom du salon
 * @return -1 si erreur, 0 sinon
 */
int join_room (user u, char *room_name);

/**
 * Retire un utilisateur d'un salon
 * @param u l'utilisateur
 * @param room_name le nom du salon
 * @return -1 si erreur, 0 sinon
 */
int quit_room (user u, char *room_name);

/**
 * Supprime un salon
 * @param room_name le nom du salon
 * @return -1 si erreur, 0 sinon
 */
int delete_room (char *room_name);

#endif
