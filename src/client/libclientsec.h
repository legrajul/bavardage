#ifndef _LIBCLIENTSEC_H
#define _LIBCLIENTSEC_H

#define CertFile "toto_certif.pem"
#define KeyFile "toto.pem"

#include "libclient.h"

#include "../common/SocketTCP.h"

#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

int connect_with_authentication (char *chatservaddr, int chatservport,
		char *secservaddr, int secservport);

int disconnect_servers ();

int generate_private_key ();

int generate_certificate_request (char *common_name, char *locality,
		char *country, char* organization, char *email_address);

int get_certificate (char *pkiaddr);

int send_message_sec (const char *mess, char **error_mess);

int set_certif_filename (const char *f);

int set_private_key_filename (const char *f);

int disconnect_sec ();

char *create_challenge_sec (const char *data);

int send_command_sec ();

int extract_code_sec (const char *str);

int aes_init (unsigned char *key, unsigned char *iv, EVP_CIPHER_CTX *e_ctx,
		EVP_CIPHER_CTX *d_ctx);

unsigned char *aes_encrypt (EVP_CIPHER_CTX *e, unsigned char *plaintext,
		int *len);

unsigned char *aes_decrypt (EVP_CIPHER_CTX *e, unsigned char *ciphertext,
		int *len);

#endif
