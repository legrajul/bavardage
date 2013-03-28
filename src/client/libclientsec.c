/* OpenSSL headers */
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>

#include "libclientsec.h"
#include "../common/commonsec.h"

char *private_key_filename;
char *certif_request_filename;
char *certif_filename;

SocketTCP *secure_socket;

int is_connected = 0;

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

int setup_ctx () {
    printf ("BEGIN setup_ctx\n");

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

    ctx = initialize_ctx (CertFile, KeyFile, "toto");

    ssl = SSL_new(ctx);
    sbio = BIO_new_socket(secure_socket->socket, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    if(SSL_connect(ssl)<=0)
      berr_exit("SSL connect error");

    printf("You can now send commands and messages\n");
    //(void) signal(SIGINT, my_sigaction);


    //TODO ceci est du code de test

    int ssl_bytes_written = SSL_write (ssl, "salut", 5);
    printf ("SSL_bytes_written = %d\n", ssl_bytes_written);

    SSL_write (ssl, "salut", 5);

    printf ("END connect_secure_socket\n");
    return 0;
}

int connect_with_authentication (char *chatservaddr, int chatservport, char *login,
                                 char *secservaddr, int secservport) {
    printf ("BEGIN connect_with_authentication\n");
    connect_socket (chatservaddr, chatservport);
    connect_secure_socket (secservaddr, secservport);

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
