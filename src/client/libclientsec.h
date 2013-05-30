#ifndef _LIBCLIENTSEC_H
#define _LIBCLIENTSEC_H

#define CertFile "toto_certif.pem"
#define KeyFile "toto.pem"

#define AES_BLOCK_SIZE 256
#define MAX_CIPHERED_SIZE 2024

#include "libclient.h"

#include "../common/SocketTCP.h"
#include "../common/common.h"
#include "../common/commonsec.h"
#include "../common/room_manager.h"
#include "../common/room_manager_sec.h"

#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

typedef struct KEY_IV {
	unsigned char key[32];
	unsigned char iv[32];
}*key_iv;


/**
 *      Récupération du mot de passe de la clé privé
 *      @param password mot de passe
 */
void set_private_key_password (char *password);

/**
 *      Connexion avec authentification
 *      @param chatservaddr adresse du serveur
 *      @param chatservport port du serveur
 * 		@param secservaddr adresse du serveur sécurisé
 * 		@param secservport port du serveur sécurisé
 */
int connect_with_authentication (char *chatservaddr, int chatservport,
		char *secservaddr, int secservport);

/**
 *      Déconnexion du client des serveurs
 */
int disconnect_servers ();

/**
 *      Envoi d'un message au serveur sécurisé
 *      @param mess message à envoyer
 *      @param error_mess message d'erreur
 * 		@return 0
 */
int send_message_sec (const char *mess, char **error_mess);

/**
 *      Récupération du nom du certificat pour le stocker dans une variable
 *      @param f nom du certificat
 */
int set_certif_filename (const char *f);

/**
 *      Récupération du nom du certificat pour le stocker dans une variable
 *      @param f nom du certificat
 */
int set_private_key_filename (const char *f);

/**
 *      Récupération du nom du certificat racine pour le stocker dans une variable
 *      @param filename nom du certificat racine
 */
void set_root_certif_filename (const char *filename);

/**
 *      Déconnexion du client du serveur sécurisé
 */
int disconnect_sec ();

char *create_challenge_sec (const char *data);

/**
 *      Envoi des commandes sécurisés au serveur
 *      @return 0 ou -1 en cas d'erreur ou 1 lorsque le message àenvoyer est vide
 */
int send_command_sec ();

int extract_code_sec (const char *str); 

/**
 *      Initialisation du contexte de chiffrement avec AES
 *      @param key clé de chiffrement
 *      @param iv vecteur d'initialisation
 *      @param e_ctx contexte de chiffrement
 *      @return d_ctx contexte de déchiffrement
 */
int aes_init (unsigned char *key, unsigned char *iv, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx);

/**
 *      Chiffrement d'un texte avec AES
 *      @param k clé de chiffrement
 *      @param plaintext texte clair à chiffrer
 *      @param len longueur du texte clair
 */
char *aes_encrypt (keys k, char *plaintext, int *len);

/**
 *      Déchiffrement d'un texte avec AES
 *      @param k clé de chiffrement
 *      @param ciphertext texte chiffré à déchiffrer
 *      @param len longueur du texte chiffré
 */
char *aes_decrypt (keys k, char *ciphertext, int *len);

/**
 *      Déchiffrement d'un texte envoyé dans un salon
 *      @param room_name nom du salon
 *      @param ciphered texte chiffré
 *      @param len longueur du texte chiffré
 */
char *decrypt (char *room_name, char *ciphered, int ciphered_size);

/**
 * Renvoie un message envoyé par le serveur sécurisé
 * @param m le message envoyé
 * @return -1 en cas d'erreur, 0 sinon
 */
int receive_message_sec(message *m);

#endif
