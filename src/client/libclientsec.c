/* OpenSSL headers */
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>

#include "libclientsec.h"
#include "libclient.h"

#include "../common/common.h"
#include "../common/commonsec.h"

char *private_key_filename;
char *certif_request_filename;
char *certif_filename;

SocketTCP *secure_socket;

int is_connected = 0;

SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;

message *msg;

extern char *login, **tab_string;

int set_certif_filename (const char *certif_f) {
    certif_filename = strdup (certif_f);
}

int set_certif_request_filename (const char *certif_req_f) {
    certif_request_filename = strdup (certif_req_f);
}

int set_private_key_filename (const char *private_key_f) {
    private_key_filename = strdup (private_key_f);
}

int connect_secure_socket(const char *addr, const int port, const char *password) {
    printf ("BEGIN connect_secure_socket\n");
    int co;
    if ((secure_socket = creerSocketTCP()) == NULL) {
        return -1;
    }
    printf("Please wait for connecting the server...\n");
    if ((co = connectSocketTCP(secure_socket, addr, port)) == -1) {
        return -1;
    }    

    ctx = initialize_ctx (certif_filename, private_key_filename, password);

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
    connect_secure_socket (secservaddr, secservport, "toto");

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


int extract_code_sec(const char *str) {
    char *command = NULL;
    command = str_sub(str, 1, strlen(str));

    if (strcmp(command, "CREATE_ROOM_SEC") == 0) {
        return CREATE_ROOM_SEC;
    } else if (strcmp(command, "DELETE_ROOM_SEC") == 0) {
        return DELETE_ROOM_SEC;
    } else if (strcmp(command, "DISCONNECT_SEC") == 0) {
        return DISCONNECT_SEC;
    } else if (strcmp(command, "CONNECT_SEC") == 0) {
        return CONNECT_SEC;
    } else if (strcmp(command, "QUIT_ROOM_SEC") == 0) {
        return QUIT_ROOM_SEC;
    } else if (strcmp(command, "JOIN_ROOM_SEC") == 0) {
        return JOIN_ROOM_SEC;
    }

    return -1;
}

int send_command_sec () {
    if (secure_socket == NULL) {
        return -1;
    }
    if (msg == NULL) {
        msg = (message*) malloc(sizeof(message));
    }
    if (login != NULL)
        strcpy(msg->sender, login);
    else
        return -1;
    if (SSL_write(ssl, (char *) msg, sizeof(message)) < 0) {
        return (1);
    }

    return 0;
}


int send_message_sec (const char *mess, char **error_mess) {
    int code;
    char buffer[20 + MAX_NAME_SIZE + MAX_MESS_SIZE] = "";
    strcpy(buffer, mess);
    buffer[strlen(buffer)] = '\0';

    msg = (message*) malloc(sizeof(message));

    if (mess[0] == '/') {
        code = extract_code_sec(strtok(strdup(buffer), " "));
        if (code == -1) {
            return -2;
        }
        msg->code = code;
        char *tmp, buff[MAX_MESS_SIZE] = "";
        int i;

        switch (code) {
        case CONNECT_SEC:   // Cas d'une demande de connexion
            tmp = strtok(NULL, " ");
            if (tmp != NULL) {
                login = strdup(tmp);
            }
            if (login == NULL) {
                printf("login null\n");
                return -1;
            } else {
                strcpy(msg->sender, login);
                return send_command_sec();
            }
            break;

        case DISCONNECT_SEC:        // Cas d'une demande de déconnexion
            strcpy(msg->sender, login);
            // disconnect();
            break;

        case CREATE_ROOM_SEC:       // Cas d'une demande de création de Salon
            tmp = strtok(NULL, " ");
            if (tmp != NULL) {
                strcpy(msg->content, tmp);
            } else {
                *error_mess = strdup ("CREATE_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            return send_command_sec();
            break;
        case DELETE_ROOM_SEC:
            tmp = strtok(NULL, " ");
            if (tmp != NULL) {
                strcpy(msg->content, tmp);
            } else {
                *error_mess = strdup ("DELETE_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            return send_command_sec();
            break;
        case QUIT_ROOM_SEC:         // Cas d'une demande pour quitter une room
            tmp = strtok(NULL, " ");
            if (tmp != NULL) {
                strcpy(msg->content, tmp);
            } else {
                *error_mess = strdup ("QUIT_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            strcpy(msg->sender, login);
            return send_command_sec();

            break;

        case JOIN_ROOM_SEC:
            tmp = strtok(NULL, " ");
            if (tmp != NULL) {
                strcpy(msg->content, tmp);
            } else {
                *error_mess = strdup ("JOIN_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            return send_command_sec();
            break;

        }
    }
    return 0;
}
