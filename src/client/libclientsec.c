/* OpenSSL headers */
#include <string.h>

#include "libclientsec.h"
#include "libclient.h"

#include "../common/common.h"
#include "../common/commonsec.h"
#include "../common/room_manager.h"
#include "../common/room_manager_sec.h"

#define CAFILE "root.pem"
#define CADIR NULL
#define CERTFILE "toto_certif.pem"
#define KEYFILE "toto_key.pem"

char *private_key_filename = KEYFILE;
char *certif_request_filename;
char *certif_filename = CERTFILE;

SocketTCP *secure_socket;

int is_connected = 0;
SSL_CTX *ctx;
SSL *ssl;
BIO *sbio;
message *msg;
int debug = 0;
int is_room_create = 0;

extern char *login, **tab_string;
//partie test échange sec
unsigned int salt[] = {12345, 54321};
int leng;

//fin partie test échange

int set_certif_filename (const char *certif_f) {
    certif_filename = strdup (certif_f);
}

int set_certif_request_filename (const char *certif_req_f) {
    certif_request_filename = strdup (certif_req_f);
}

int set_private_key_filename (const char *private_key_f) {
    private_key_filename = strdup (private_key_f);
}

//modif
char *pass = "";
int password_cb(char *buf,int num, int rwflag,void *userdata) {
    if(num<strlen(pass)+1)
        return(0);

    strcpy(buf,pass);
    return(strlen(pass));
}

void set_private_key_password (char *password) {
    pass = strdup (password);
}

SSL_CTX *setup_client_ctx (void) {
    //SSL_CTX *ctx;
    ctx = SSL_CTX_new (SSLv23_method ());
    SSL_CTX_set_default_passwd_cb (ctx, password_cb);
    if (SSL_CTX_load_verify_locations (ctx, CAFILE, CADIR) != 1)
        fprintf (stderr, "Error loading CA file and/or directory\n");
    if (SSL_CTX_set_default_verify_paths (ctx) != 1)
        fprintf (stderr, "Error loading default CA file and/or directory\n");
    if (SSL_CTX_use_certificate_chain_file (ctx, certif_filename) != 1)
        fprintf (stderr, "Error loading certificate from file\n");
    if (SSL_CTX_use_PrivateKey_file (ctx, private_key_filename, SSL_FILETYPE_PEM) != 1)
        fprintf (stderr, "Error loading private key from file\n");
    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER, verify_callback);
    SSL_CTX_set_verify_depth (ctx, 4);
    return ctx;
}

//fin modif

int connect_secure_socket (const char *addr, const int port) {
    printf ("BEGIN connect_secure_socket\n");
    int co;
    if ((secure_socket = creerSocketTCP ()) == NULL) {
        fprintf (stderr, "Failed to create SocketTCP\n");
        return -1;
    }
    printf ("Please wait for connecting the server...\n");
    if ((co = connectSocketTCP (secure_socket, addr, port)) == -1) {
        fprintf (stderr, "Failed to connect the SocketTCP to %s:%d\n", addr, port);
        return -1;
    }

    //ctx = initialize_ctx (certif_filename, private_key_filename, password);
    ctx = setup_client_ctx ();
    ssl = SSL_new (ctx);
    sbio = BIO_new_socket (secure_socket->socket, BIO_NOCLOSE);
    SSL_set_bio (ssl, sbio, sbio);

    SSL_check_private_key (ssl);

    if (SSL_connect (ssl) <= 0)
        berr_exit ("SSL connect error");

    //long err;
    // modif a refaire
    /*if ((err = post_connection_check(ssl, "localhost")) != X509_V_OK) {
      fprintf(stderr, "-Error: peer certificate: %s\n", X509_verify_cert_error_string(err));
      //int_error("Error checking SSL object after connection");
      }*/

    printf ("SSL Connection opened\n");
    // pour le disconect
    //SSL_clear(ssl);
    //SSL_free(ssl);
    //SSL_CTX_free(ctx);
    // fin modif

    printf ("You can now send commands and messages\n");
    //(void) signal(SIGINT, my_sigaction);

    printf ("END connect_secure_socket\n");
    return 0;
}

int connect_with_authentication (char *chatservaddr, int chatservport,
                                 char *secservaddr, int secservport) {
    printf ("BEGIN connect_with_authentication\n");
    connect_socket (chatservaddr, chatservport);
    connect_secure_socket (secservaddr, secservport);

    printf ("END connect_secure_socket\n");
}

int disconnect_servers () {
    disconnect ();
    disconnect_sec ();
}

int generate_private_key () {
    //TODO
}

int generate_certificate_request (char *common_name, char *locality,
                                  char *country, char* organization, char *email_address) {
    //TODO
}

int get_certificate (char *pkiaddr) {
    //TODO
}

int aes_init (unsigned char *key, unsigned char *iv, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx) {

    EVP_CIPHER_CTX_init (e_ctx);
    EVP_EncryptInit_ex (e_ctx, EVP_aes_256_cbc (), NULL, key, iv);
    EVP_CIPHER_CTX_init (d_ctx);
    EVP_DecryptInit_ex (d_ctx, EVP_aes_256_cbc (), NULL, key, iv);

    return 0;
}

char *aes_encrypt (unsigned char *key, unsigned char *iv, char *plaintext, int *len) {
    printf ("AES_ENCRYPT BEGIN plaintext = <%s>, len = <%d>\n", plaintext, *len);
    EVP_CIPHER_CTX e_ctx, d_ctx;
    aes_init (key, iv, &e_ctx, &d_ctx);
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
    char *ciphertext = malloc (c_len);

    EVP_EncryptInit_ex (&e_ctx, NULL, NULL, NULL, NULL);
    EVP_EncryptUpdate (&e_ctx, ciphertext, &c_len, plaintext, *len);
    EVP_EncryptFinal_ex (&e_ctx, ciphertext + c_len, &f_len);

    *len = c_len + f_len;
    printf ("AES ENCRYPT END len = <%d>, ret = <%s>\n", *len, ciphertext);
    return ciphertext;
}

char *aes_decrypt (unsigned char *key, unsigned char *iv, char *ciphertext,
                   int *len) {
    printf ("AES_DECRYPT BEGIN ciphertext = <%s>, len = <%d>\n", ciphertext, *len);
    EVP_CIPHER_CTX e_ctx, d_ctx;
    aes_init (key, iv, &e_ctx, &d_ctx);
    int p_len = *len, f_len = 0;
    char *plaintext = malloc (p_len + AES_BLOCK_SIZE);

    EVP_DecryptInit_ex (&d_ctx, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate (&d_ctx, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex (&d_ctx, plaintext + p_len, &f_len);

    *len = p_len + f_len;
    printf ("AES DECRYPT END len = <%d>, ret = <%s>\n", *len, plaintext);
    return plaintext;
}

int receive_message_sec(message *m) {
    int ret = SSL_read (ssl, (char *) m, sizeof(message));
    if (ret == 0) {
        return -1;
    } else {
        return 0;
    }
}

int extract_code_sec (const char *str) {
    char *command = NULL;
    command = str_sub (str, 1, strlen (str));

    if (strcmp (command, "CREATE_ROOM_SEC") == 0) {
        return CREATE_ROOM_SEC;
    } else if (strcmp (command, "DELETE_ROOM_SEC") == 0) {
        return DELETE_ROOM_SEC;
    } else if (strcmp (command, "DISCONNECT_SEC") == 0) {
        return DISCONNECT_SEC;
    } else if (strcmp (command, "CONNECT_SEC") == 0) {
        return CONNECT_SEC;
    } else if (strcmp (command, "QUIT_ROOM_SEC") == 0) {
        return QUIT_ROOM_SEC;
    } else if (strcmp (command, "JOIN_ROOM_SEC") == 0) {
        return JOIN_ROOM_SEC;
    } else if (strcmp (command, "DEL_ACCOUNT_SEC") == 0) {
        return DEL_ACCOUNT_SEC;
    } else if (strcmp (command, "MP_SEC") == 0) {
        return MP_SEC;
    } else if (strcmp(command, "CONNECT_KO_SEC_OK") == 0) {
        return CONNECT_KO_SEC_OK;
    } else if (strcmp (command, "MESSAGE") == 0) {
        return MESSAGE;
    } else if (strcmp(command, "CONNECT_OK") == 0) {
        return CONNECT_OK;
    } else if (strcmp(command, "ACCEPT_JOIN_ROOM_SEC") == 0) {
	   return ACCEPT_JOIN_ROOM_SEC;
    } else if (strcmp(command, "REFUSE_JOIN_ROOM_SEC") == 0) {
	   return REFUSE_JOIN_ROOM_SEC;
    } else if (strcmp(command, "EJECT_FROM_ROOM_SEC") == 0) {
        return EJECT_FROM_ROOM_SEC;
    }
    return -1;
}


int send_command_sec () {
    printf ("DEBUG_SEND_COMMAND-%d\n", debug++);
    if (secure_socket == NULL) {
        return -1;
    }
    if (msg == NULL) {
        msg = (message*) malloc (sizeof(message));
    }
    if (login != NULL)
        strcpy (msg->sender, login);
    else
        return -1;
    printf ("libclientsec.c: send_command: DEBUG_SEND_COMMAND-%d\n", debug++);
    printf ("libclientsec.c: send_command: SIZEOF_MESSAGE-%ld\n", sizeof(message));
    printf ("libclientsec.c: send_command: MSG_SENDER-%s\n", msg->sender);
    printf ("libclientsec.c: send_command: msg->code(dans send_command_sec): %d\n", msg->code);
    printf ("libclientsec.c: send_command: msg->sender: %s\n", msg->sender);
    printf ("libclientsec.c: send_command: msg->content: %s\n", msg->content);
    if (SSL_write (ssl, (char *) msg, sizeof(message)) < 0) {
        return (1);
    } else {
        printf ("DEBUG_SEND_COMMAND--%d\n", debug++);
    }

    return 0;
}

char *create_challenge_sec (const char *data) {
    printf ("libclientsec.c: create_challenge: BEGIN - create_challenge_sec with data = <%s> (%d char)\n", data, strlen (data));
    uint8_t *encryptedBytes = NULL;
    //const char* data = "Data to enrypt";
    char *private_key_file_name = KEYFILE;

    FILE *fp = fopen (private_key_file_name, "r");
    RSA *rsa = RSA_new ();

    PEM_read_RSAPrivateKey (fp, &rsa, 0, NULL);

    size_t encryptedBytesSize = RSA_size (rsa);

    encryptedBytes = malloc (encryptedBytesSize * sizeof(uint8_t));
    memset ((void *) encryptedBytes, 0x0, encryptedBytesSize);
    fclose (fp);

    int result = RSA_private_encrypt (strlen (data), data, encryptedBytes, rsa,
                                      RSA_PKCS1_PADDING);
    printf ("RSA_size-%i\n", RSA_size (rsa));
    return encryptedBytes;
}

int send_message_sec (const char *mess, char **error_mess) {
    printf("DEBUT send_message_sec\n");
    int code;
    char buffer[20 + MAX_NAME_SIZE + MAX_MESS_SIZE] = "";
    char *ciphermess;
    EVP_CIPHER_CTX en;
    EVP_CIPHER_CTX de;
    int lenght;
    key_iv keyiv;
    unsigned char key[32], iv[32];
    unsigned char *keydata="test";
    key_iv ki;

    char text[MAX_MESS_SIZE] = "";
    strcpy(buffer, mess);
    buffer[strlen (buffer)] = '\0';

    msg = (message*) malloc (sizeof(message));
    printf ("DEBUG_SEND_MESS-%d\n", debug++);
    if (mess[0] == '/') {
        code = extract_code_sec (strtok (strdup (buffer), " "));
        if (code == -1) {
            return -2;
        }
        msg->code = code;

        printf("%d\n",code);
        char *tmp, *pass, buff[MAX_MESS_SIZE] = "", conn[MAX_MESS_SIZE] = "";
        uint8_t *challenge;
        int i;
        printf ("libclientsec.c: send_mess: DEBUG_SEND_MESS-%d\n", debug++);
        switch (code) {
        case CONNECT_OK:
            printf("--------------------------DEBUT CONNECT_OK libclientsec ------------------------\n");
            msg->code = CONNECT_OK;
            send_command_sec();
            break;
        case CONNECT_KO_SEC_OK:
            printf("DEBUT CONNECT_KO_SEC_OK\n");
            /*msg->code = DEL_ACCOUNT_SEC;
              strcpy(msg->sender, login);
              strcpy(msg->content, login);
              send_command_sec();*/
            disconnect_sec();
            break;
        case CONNECT_SEC:   // Cas d'une demande de connexion
			is_connected = 1;
            printf("--------------------------DEBUT CONNECT_SEC libclientsec ------------------------\n");
            printf("libclientsec.c: send_mess: msg-code(debut swicth): %d\n", msg->code);
            //strcpy(conn, "/CONNECT ");
            printf ("DEBUG_SEND_MESS-%d\n", debug++);
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                login = strdup (tmp);
            }
            printf("libclientsec.c: send_mess: AFTER send_message\n");

            //printf("CHALLENGE_SIZE-%s\n", RSA_size(rsa));

            //strcat(conn, login);
            //send_message (conn, &error_mess);
            msg->code = CONNECT_SEC;
            printf ("libclientsec.c: send_mess: DEBUG_SEND_MESS-%d\n", debug++);
            if (login == NULL) {
                printf ("login null\n");
                return -1;
            } else {
                printf ("libclientsec.c: send_mess: LOGIN-%s\n", login);
                strcpy (msg->sender, login);
                return send_command_sec ();
            }
            break;

        case DEL_ACCOUNT_SEC:
            printf("libclientsec.c: case del account sec\n");
            //~ tab_string = create_table_param(buffer);
            //~ if (len (tab_string) < 3) {
            //~ *error_mess = strdup ("use: /DEL_ACCOUNT_SEC user password\n");
            //~ return -3;
            //~ }

            tmp = strtok (NULL, " ");

            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("use: /DEL_ACCOUNT_SEC user \n");
                return -3;
            }
            send_command_sec ();
            disconnect_sec ();
            break;

        case DISCONNECT_SEC:        // Cas d'une demande de déconnexion
            if (is_connected == 0)
				printf("you're not connected yet\n");
			else {
	            strcpy (msg->sender, login);
	            strcpy(conn, "/DISCONNECT ");
	            strcat(conn, login);
	            send_message (conn, &error_mess);
	            msg->code = DISCONNECT_SEC;
	            disconnect_sec ();
	            printf("libclientsec.c: send_mess: FIN CASE disconnect_sec\n");
			}
            break;

        case CREATE_ROOM_SEC:       // Cas d'une demande de création de Salon
            tmp = strtok (NULL, " ");
            printf("tmp = %s \n", tmp);
            if (msg == NULL) {
                printf ("msg is null /!\\\n");
            }
            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("CREATE_ROOM a besoin d'un paramètre\n");
                return -3;
            }

            add_room (msg->content, NULL);
            strcpy(conn, "/CREATE_ROOM ");
            strcat(conn, msg->content);
            send_message (conn, &error_mess);

            msg->code = CREATE_ROOM_SEC;
            return send_command_sec ();


            //msg->code = CREATE_ROOM_SEC;

            //tmp = strtok (NULL, " ");
            //if (tmp != NULL) {
            /*    strcpy (msg->content, tmp);
                  } else {
                  *error_mess = strdup ("CREATE_ROOM a besoin d'un paramètre\n");
                  return -3;
                  }
                  return send_command_sec ();*/
            break;

        case DELETE_ROOM_SEC:
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("DELETE_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            strcpy(text, "/DELETE_ROOM ");
            strcat(text, msg->content);
            printf("message envoyé a liblcient send_message: <%s>\n", text);
            send_message(text, &error_mess);
            if (error_mess != NULL)
                printf("libclientsec - error_mess: <%s>\n", error_mess);
            msg->code = DELETE_ROOM_SEC;
            return send_command_sec ();
            break;

        case QUIT_ROOM_SEC:         // Cas d'une demande pour quitter une room
            printf("Debut quit room \n");
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("QUIT_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            strcpy (msg->sender, login);
            printf("Fin quit room \n");
            return send_command_sec ();
            
            break;

	case ACCEPT_JOIN_ROOM_SEC:
	    tab_string = create_table_param(buffer);
            if (len (tab_string) < 3) {
                *error_mess = strdup ("ACCEPT_JOIN_ROOM_SEC doit avoir 2 paramètres : /ACCEPT_JOIN_ROOM_SEC salon utilisateur\n");
                return -3;
            }
	    strcpy (msg->content, tab_string[1]);
	    strcpy (msg->receiver, tab_string[2]);
	    msg->code = ACCEPT_JOIN_ROOM_SEC;
            return send_command_sec ();
	    break;

    case EJECT_FROM_ROOM_SEC:
        tab_string = create_table_param(buffer);
        if (len (tab_string) < 3) {
            *error_mess = strdup ("EJECT_FROM_ROOM_SEC doit avoir 2 paramètres : /EJECT_FROM_ROOM_SEC salon utilisateur\n");
            return -3;
        }
        strcpy (msg->content, tab_string[1]);
        strcpy (msg->receiver, tab_string[2]);
        msg->code = EJECT_FROM_ROOM_SEC;
        return send_command_sec ();
        break;
	case REFUSE_JOIN_ROOM_SEC:
	    tab_string = create_table_param(buffer);
            if (len (tab_string) < 3) {
                *error_mess = strdup ("REFUSE_JOIN_ROOM_SEC doit avoir 2 paramètres : /MESSAGE salon utilisateur\n");
                return -3;
            }
	    strcpy (msg->content, tab_string[1]);
	    strcpy (msg->receiver, tab_string[2]);
	    msg->code = REFUSE_JOIN_ROOM_SEC;
            return send_command_sec ();
	    break;

        case JOIN_ROOM_SEC:
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("JOIN_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            return send_command_sec ();
            break;

        case MESSAGE:  // Cas d'envoi de message
            tab_string = create_table_param(buffer);
            if (len (tab_string) < 3) {
                *error_mess = strdup ("MESSAGE doit avoir 2 paramètres : /MESSAGE salon mon super message\n");
                return -3;
            }
            strcpy(msg->receiver, tab_string[1]);
            for (i = 2; i < len(tab_string); i++) {
                strcat(buff, tab_string[i]);
                strcat(buff, " ");
            }
            if(is_room_used(msg->receiver) == 0) {
                strcpy (msg->content, buff);
                printf("message non chiffré\n");
            }
            else {
                lenght = strlen(buff) + 1;
                //keyiv = malloc(sizeof (struct KEY_IV));
                keyiv = get_keyiv_in_room(msg->receiver);
                ciphermess = aes_encrypt(keyiv->key, keyiv->iv, (char *)buff, &lenght);
                strcpy(msg->content, ciphermess);
                //free(ciphermess);
                //free(keyiv);
            }
            free(tab_string);
            return send_command();
            break;

        case MP_SEC:  // Cas d'envoi de message prive
            tab_string = create_table_param(buffer);
            if (len (tab_string) < 3) {
                *error_mess = strdup ("MP_SEC doit avoir 2 paramètres : /MP toto mon super message privé\n");
                return -3;
            }
            strcpy(msg->receiver, tab_string[1]);
            strcpy (buff, "");
            for (i = 2; i < len(tab_string); i++) {
                strcat(buff, tab_string[i]);
                strcat(buff, " ");
            }

            if (lenght > MAX_MESS_SIZE) {
                *error_mess = strdup ("ce message est trop long pour être envoyé chiffré\n");
                return -3;
            }
            if (is_room_used(msg->receiver)==1) {
				lenght = strlen(buff) + 1;
				keyiv = get_keyiv_in_room(msg->receiver);
				ciphermess = aes_encrypt(keyiv->key, keyiv->iv, (char *)buff, &lenght);
				strcpy(msg->content,ciphermess);
				msg->code=MP;
				return send_command();			
			} else {
				msg->code=MP_SEC;
				strcpy(msg->content,buff);
				return send_command_sec();
			}
			free(tab_string);
            break;
        }
    }
    return 0;
}

int disconnect_sec () {
    if (secure_socket != NULL && msg != NULL) {
        msg->code = DISCONNECT_SEC;
        send_command_sec ();
    }
    return 0;
}

