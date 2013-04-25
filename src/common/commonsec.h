#ifndef _COMMONSEC_H
#define _COMMONSEC_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

//modif
//#define int_error(msg) handle_error(__FILE__, __LINE_ _, msg)
#include <pthread.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>

#define MUTEX_TYPE pthread_mutex_t
#define MUTEX_SETUP(x) pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x) pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x) pthread_mutex_unlock(&(x))
#define THREAD_ID pthread_self()


// fin modif

#include <openssl/ssl.h>

#define CREATE_ROOM_SEC     111
#define DELETE_ROOM_SEC     211
#define QUIT_ROOM_SEC 	311
#define JOIN_ROOM_SEC	411
#define DISCONNECT_SEC	511
#define CONNECT_SEC		611

#define BUFSIZZ 1024
#define AES_BLOCK_SIZE 256

extern BIO *bio_err;
int berr_exit (char *string);
int err_exit (char *string);

SSL_CTX *initialize_ctx (char *certiffile, char *keyfile, char *password);
void destroy_ctx (SSL_CTX *ctx);

void handle_error (const char *file, int lineno, const char *msg);

void init_OpenSSL (void);

int verify_callback (int ok, X509_STORE_CTX *store);

long post_connection_check (SSL *ssl, char *host);

int THREAD_setup(void);

// unsigned long id_function(void);
// void locking_function(int mode, int n, const char * file, int line);
//void handle_error(const char *file, int lineno, const char *msg);

#ifndef ALLOW_OLD_VERSIONS
#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
#error "Must use OpenSSL 0.9.6 or later"
#endif
#endif

#endif

