#ifndef _SECURE_SERVER_H
#define _SECURE_SERVER_H

#define CertFile "bavardageserversec_certif.pem"
#define KeyFile "bavardageserversec.pem"

/* OpenSSL headers */
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include "../common/room_manager.h"

void *handle_connexion(void *param);

/**
 *   Crée la socket serveur
 *   Se met en écoute pour attendre la connexion d'un client
 *   Crée un thread
 *      @param addr adresse ip du serveur
 *   @param port port du serveur
 *   @return -1 si erreur de création de socket ou de thread
 */

int start_listening(const char *addr, int port);

/**
 *   Initie un thread
 *   Crée un thread
 *   Détache le thread
 *   Détruit le thread
 *      @param socket socket client
 */

void new_thread(SocketTCP *socket);

/**
 * Vide la structure message
 * @param m structure message
 */
int clear_message(message *m);

/**
 * Créer le salon d'accueil
 * @return -1 si erreur, 0 sinon
 */
int create_main_room();

/**
 * Créer un salon
 * @param room_name le nom du salon
 * @return -1 si erreur, 0 sinon
 */
int create_room(char *room_name); 

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

void randomString(char *str, size_t n);

unsigned int * randomInt();

keys gen_keys ();

#endif
