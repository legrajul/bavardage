/* OpenSSL headers */
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>

#include "libclientsec.h"

char *private_key_filename;
char *certif_request_filename;
char *certif_filename;

SocketTCP *secure_socket;

int is_connected = 0;

SSL_METHOD *method;   /* create server instance */
SSL_CTX *ctx;         /* create context */


int setup_ctx () {
    printf ("BEGIN setup_ctx\n");
    method = SSLv3_client_method();
    int i = 1;
    printf ("debug %d\n", i++);
    ctx = SSL_CTX_new(method);
    printf ("debug %d\n", i++);
    SSL_library_init ();
    printf ("debug %d\n", i++);
    SSL_load_error_strings();
    printf ("debug %d\n", i++);
    /* set the local certificate from CertFile */
    SSL_CTX_use_certificate_file (ctx, CertFile, SSL_FILETYPE_PEM);
    printf ("debug %d\n", i++);
    /* set the private key from KeyFile */
    SSL_CTX_use_PrivateKey_file (ctx, KeyFile, SSL_FILETYPE_PEM);
    printf ("debug %d\n", i++);
    /* verify private key */
    /* if (!SSL_CTX_check_private_key(ctx)) */
        /* abort(); */

    printf ("END setup_ctx\n");
}

int connect_secure_socket(const char *addr, const int port) {
    printf ("BEGIN connect_secure_socket\n");
    int co;
    if ((secure_socket = creerSocketTCP()) == NULL) {
        return -1;
    }
    printf("Please wait for connecting the server...\n");
    if ((co = connectSocketTCP(secure_socket, addr, port)) == -1) {
        return -1;
    }

    printf("You can now send commands and messages\n");
    //(void) signal(SIGINT, my_sigaction);

    printf ("END connect_secure_socket\n");
    return 0;
}

int connect_with_authentication (char *chatservaddr, int chatservport, char *login,
                                 char *secservaddr, int secservport) {
    printf ("BEGIN connect_with_authentication\n");
    connect_socket (chatservaddr, chatservport);
    connect_secure_socket (secservaddr, secservport);

    SSL *ssl = SSL_new(ctx);  /* get new SSL state with context */
    SSL_set_fd(ssl, secure_socket->socket);    /* set connection to SSL state */
    SSL_accept(ssl);           /* start the handshaking */

    SSL_write (ssl, "test", 5);

    printf ("END connect_secure_socket\n");
}

int disconnect_servers () {
    //TODO
}

int generate_private_key () {
    //TODO
}

int generate_certificate_request (char *common_name, char *locality, char *country,
                                  char* organization, char *email_address) {
    //TODO
}

int get_certificate (char *pkiaddr) {
    //TODO
}
