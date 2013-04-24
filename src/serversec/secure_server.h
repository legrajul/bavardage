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


void randomString(char *str, size_t n);

void gen_keyiv(key_iv *keyiv, unsigned char *key_data, int key_data_len, unsigned char *salt);

#endif
