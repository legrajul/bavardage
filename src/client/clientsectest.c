#include "libclientsec.h"
#include "../common/commonsec.h"

#define CAFILE "root.pem"
#define CADIR NULL
#define CERTFILE "client.pem"
#define KEYFILE "client_key.pem"

#define CHATADDR "localhost"
#define CHATPORT 10000

#define SECADDR  "localhost"
#define SECPORT  11000

//modif

SSL_CTX *setup_client_ctx(void) {
	SSL_CTX *ctx;
	ctx = SSL_CTX_new(SSLv23_method());
	if (SSL_CTX_load_verify_locations(ctx, CAFILE, CADIR) != 1)
		fprintf(stderr, "Error loading CA file and/or directory\n");
	if (SSL_CTX_set_default_verify_paths(ctx) != 1)
		fprintf(stderr, "Error loading default CA file and/or directory\n");
	if (SSL_CTX_use_certificate_chain_file(ctx, CERTFILE) != 1)
		fprintf(stderr, "Error loading certificate from file\n");
	if (SSL_CTX_use_PrivateKey_file(ctx, KEYFILE, SSL_FILETYPE_PEM) != 1)
		fprintf(stderr, "Error loading private key from file\n");
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
	SSL_CTX_set_verify_depth(ctx, 4);
	return ctx;
}

//fin modif

int main (int argc, char *argv[]) {

	init_OpenSSL();
	//seed_prng();
	SSL_CTX *ctx;
	ctx = setup_client_ctx();

    //set_private_key_filename ("toto.pem");
    //set_certif_filename ("toto_certif.pem");
    connect_with_authentication (CHATADDR, CHATPORT, argv[1], SECADDR, SECPORT);
    char *err;
    send_message_sec("/CONNECT_SEC tototo", &err);
    return 0;
}
