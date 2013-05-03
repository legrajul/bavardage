#include "commonsec.h"
#include "common.h"
#include <openssl/err.h>
#include <strings.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/conf.h>

BIO *bio_err = 0;
char *pass;
int password_cb (char *buf, int num, int rwflag, void *userdata);
void sigpipe_handle (int x);
MUTEX_TYPE *mutex_buf = NULL;

/* A simple error and exit routine*/
int err_exit (char *string) {
    fprintf (stderr, "%s\n", string);
    exit (0);
}

/* Print SSL errors and exit*/
int berr_exit (char *string) {
    BIO_printf (bio_err, "%s\n", string);
    ERR_print_errors (bio_err);
    exit (0);
}


void destroy_ctx (SSL_CTX *ctx) {
    SSL_CTX_free (ctx);
}

unsigned long id_function(void) {
    return ((unsigned long)THREAD_ID);
}

void locking_function(int mode, int n, const char * file, int line) {
    if (mode & CRYPTO_LOCK)
        MUTEX_LOCK(mutex_buf[n]);
    else
        MUTEX_UNLOCK(mutex_buf[n]);
}

int THREAD_setup(void) {
    int i;
    mutex_buf = (MUTEX_TYPE *)malloc(CRYPTO_num_locks() *
                                     sizeof(MUTEX_TYPE));
    if (!mutex_buf)
        return 0;
    for (i = 0; i < CRYPTO_num_locks(); i++)
        MUTEX_SETUP(mutex_buf[i]);
    CRYPTO_set_id_callback(id_function);
    CRYPTO_set_locking_callback(locking_function);
    return 1;
}

void handle_error(const char *file, int lineno, const char *msg) {
    fprintf(stderr, "** %s:%i %s\n", file, lineno, msg);
    ERR_print_errors_fp(stderr);
    exit(-1);
}

void init_OpenSSL(void) {
    if (!THREAD_setup() || ! SSL_library_init()) {
        fprintf(stderr, "** OpenSSL initialization failed!\n");
        exit(-1);
    }
    SSL_load_error_strings();
}

int verify_callback(int ok, X509_STORE_CTX *store) {
    char data[256];
    if (!ok) {
        X509 *cert = X509_STORE_CTX_get_current_cert(store);
        int depth = X509_STORE_CTX_get_error_depth(store);
        int err = X509_STORE_CTX_get_error(store);
        fprintf(stderr, "-Error with certificate at depth: %i\n", depth);
        X509_NAME_oneline(X509_get_issuer_name(cert), data, 256);
        fprintf(stderr, " issuer = %s\n", data);
        X509_NAME_oneline(X509_get_subject_name(cert), data, 256);
        fprintf(stderr, " subject = %s\n", data);
        fprintf(stderr, " err %i:%s\n", err, X509_verify_cert_error_string(err));
    }
    return ok;
}


long post_connection_check (SSL *ssl, char *host) {
    X509 *cert;
    X509_NAME *subj;
    char data[256];
    int extcount;
    int ok = 0;
    /* Checking the return from SSL_get_peer_certificate here is
     * strictly necessary. With our example programs, it is not
     * possible for it to return NULL. However, it is good form
     * check the return since it can return NULL if the examples
     * modified to enable anonymous ciphers or for the server to
     * require a client certificate.
     */
    if (!(cert = SSL_get_peer_certificate(ssl)) || !host)
        goto err_occured;
    if ((extcount = X509_get_ext_count(cert)) > 0) {
        int i;
        for (i = 0; i < extcount; i++) {
            const char *extstr;
            X509_EXTENSION *ext;
            ext = X509_get_ext(cert, i);
            extstr = OBJ_nid2sn(OBJ_obj2nid(X509_EXTENSION_get_object(ext)));
            if (!strcmp(extstr, "subjectAltName")) {
                int j;
                const unsigned char *data;
                STACK_OF(CONF_VALUE) *val;
                CONF_VALUE *nval;
                const X509V3_EXT_METHOD *meth;
                if (!(meth = X509V3_EXT_get(ext)))
                    break;
                data = ext->value->data;
                val = meth->i2v(meth,
                                meth->d2i(NULL, &data, ext->value->length), NULL);
                for (j = 0; j < sk_CONF_VALUE_num(val); j++) {
                    nval = sk_CONF_VALUE_value(val, j);
                    if (!strcmp(nval->name, "DNS") && !strcmp(nval->value, host)) {
                        ok = 1;
                        break;
                    }
                }
            }
            if (ok)
                break;
        }
    }
    if (!ok && (subj = X509_get_subject_name(cert)) &&
        X509_NAME_get_text_by_NID(subj, NID_commonName, data, 256) > 0) {
        data[255] = 0;
        if (strcasecmp(data, host) != 0)
            goto err_occured;
    }
    X509_free(cert);
    return SSL_get_verify_result(ssl);
 err_occured:
    printf("debut err_occured\n");
    if (cert)
        X509_free(cert);
    return X509_V_ERR_APPLICATION_VERIFICATION;
}


