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

char *private_key_filename;
char *certif_request_filename;
char *certif_filename;

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
   // printf ("BEGIN connect_secure_socket\n");
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

    printf ("You can now send commands and messages\n");
    return 0;
}

int connect_with_authentication (char *chatservaddr, int chatservport,
                                 char *secservaddr, int secservport) {
    connect_socket (chatservaddr, chatservport);
    connect_secure_socket (secservaddr, secservport);

}

int disconnect_servers () {
    disconnect ();
    disconnect_sec ();
}

int aes_init (unsigned char *key, unsigned char *iv, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx) {

    EVP_CIPHER_CTX_init (e_ctx);
    EVP_EncryptInit_ex (e_ctx, EVP_aes_256_cbc (), NULL, key, iv);
    EVP_CIPHER_CTX_init (d_ctx);
    EVP_DecryptInit_ex (d_ctx, EVP_aes_256_cbc (), NULL, key, iv);

    return 0;
}

char *aes_encrypt (unsigned char *key, unsigned char *iv, char *plaintext, int *len) {

    EVP_CIPHER_CTX e_ctx, d_ctx;
    aes_init (key, iv, &e_ctx, &d_ctx);
    int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
    char *ciphertext = malloc (c_len);

    EVP_EncryptInit_ex (&e_ctx, NULL, NULL, NULL, NULL);
    EVP_EncryptUpdate (&e_ctx, ciphertext, &c_len, plaintext, *len);
    EVP_EncryptFinal_ex (&e_ctx, ciphertext + c_len, &f_len);

    *len = c_len + f_len;

    return ciphertext;
}

char *aes_decrypt (unsigned char *key, unsigned char *iv, char *ciphertext,
                   int *len) {
  
    EVP_CIPHER_CTX e_ctx, d_ctx;
    aes_init (key, iv, &e_ctx, &d_ctx);
    int p_len = *len, f_len = 0;
    char *plaintext = malloc (p_len + AES_BLOCK_SIZE);

    EVP_DecryptInit_ex (&d_ctx, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate (&d_ctx, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex (&d_ctx, plaintext + p_len, &f_len);

    *len = p_len + f_len;

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
    } else if (strcmp(command, "HELP") == 0) {
        return HELP;
    } 
    return -1;
}


int send_command_sec () {
 
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
 
    if (SSL_write (ssl, (char *) msg, sizeof(message)) < 0) {
        return (1);
    } 

    return 0;
}

char *create_challenge_sec (const char *data) {
	printf("to know how to user an secure client, use the command: /HELP\n");
    printf ("libclientsec.c: create_challenge: BEGIN - create_challenge_sec with data = <%s> (%d char)\n", data, strlen (data));

    uint8_t *encryptedBytes = NULL;
    //const char* data = "Data to enrypt";
    char *private_key_file_name;

    FILE *fp = fopen (private_key_file_name, "r");
    RSA *rsa = RSA_new ();

    PEM_read_RSAPrivateKey (fp, &rsa, 0, NULL);

    size_t encryptedBytesSize = RSA_size (rsa);

    encryptedBytes = malloc (encryptedBytesSize * sizeof(uint8_t));
    memset ((void *) encryptedBytes, 0x0, encryptedBytesSize);
    fclose (fp);

    int result = RSA_private_encrypt (strlen (data), data, encryptedBytes, rsa,
                                      RSA_PKCS1_PADDING);
    return encryptedBytes;
}

int send_message_sec (const char *mess, char **error_mess) {
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
 
    if (mess[0] == '/') {
        code = extract_code_sec (strtok (strdup (buffer), " "));
        if (code == -1) {
            return -2;
        }
        msg->code = code;

        char *tmp, *pass, buff[MAX_MESS_SIZE] = "", conn[MAX_MESS_SIZE] = "";
        uint8_t *challenge;
        int i;
      
        switch (code) {
        case CONNECT_OK:
        
            msg->code = CONNECT_OK;
            send_command_sec();
            break;
        case CONNECT_KO_SEC_OK:
          
            /*msg->code = DEL_ACCOUNT_SEC;
              strcpy(msg->sender, login);
              strcpy(msg->content, login);
              send_command_sec();*/
            disconnect_sec();
            break;
        case CONNECT_SEC:   // Cas d'une demande de connexion
			is_connected = 1;        
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                login = strdup (tmp);
            }         

            msg->code = CONNECT_SEC;        
            if (login == NULL) {
                printf ("login null\n");
                return -1;
            } else {            
                strcpy (msg->sender, login);
                return send_command_sec ();
            }
            break;

        case DEL_ACCOUNT_SEC:        
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
			}
            break;

        case CREATE_ROOM_SEC:       // Cas d'une demande de création de Salon
			
            tmp = strtok (NULL, " ");
			if (is_room_used (tmp)) {
				*error_mess = strdup ("This room already exists\n");
				return -3;
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
            strcpy (msg->content, tmp);
            int ret = send_command_sec ();
            return ret;
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
            send_message(text, &error_mess);
            msg->code = DELETE_ROOM_SEC;
            return send_command_sec ();
            break;

        case QUIT_ROOM_SEC:         // Cas d'une demande pour quitter une room
            tmp = strtok (NULL, " ");
            if (tmp != NULL) {
                strcpy (msg->content, tmp);
            } else {
                *error_mess = strdup ("QUIT_ROOM a besoin d'un paramètre\n");
                return -3;
            }
            strcpy (msg->sender, login);
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
            }
            else {
                lenght = strlen(buff) + 1;
                keyiv = get_keyiv_in_room(msg->receiver);
                ciphermess = aes_encrypt(keyiv->key, keyiv->iv, (char *)buff, &lenght);
                strcpy(msg->content, ciphermess);
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
            
       case HELP:
			printf("\n\n----------------------------------------------------------------------------------------------------\n");
			printf("------------------------------ %sthe manual utilisation for secure user %s------------------------------\n", KRED, KWHT);
			printf("----------------------------------------------------------------------------------------------------\n\n");
			
			printf("--------------------\nCLASSIC USER can do:\n--------------------\n");			
			printf("%s/CREATE_ROOM %sroom_name               %s- create a room named <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/DELETE_ROOM %sroom_name               %s- delete a room named <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/QUIT_ROOM %sroom_name                 %s- quit a room named <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/JOIN_ROOM %sroom_name                 %s- join a room named <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/DISCONNECT                          %s- end the connection\n", KBLU, KWHT);
			printf("%s/CONNECT %suser                        %s- connect the user <user>\n", KBLU, KCYN, KWHT);
			printf("%s/MP %suser message                     %s- send a message <message> to user <user>\n\n", KBLU, KCYN, KWHT);
			
			printf("--------------------\nSECURED USER can do:\n--------------------\n");
			printf("%s/CONNECT_SEC %suser user_certif user_key    %s- connect a secure user named <user_name>\n", KBLU, KCYN, KWHT);
			printf("                                            with the certicate <user_certif> and the key <user_key>\n");
			printf("%s/CREATE_ROOM_SEC %sroom_name                %s- create a new secure room with the name <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/DELETE_ROOM_SEC %sroom_name                %s- delete a secure romm with the name <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/DISCONNECT_SEC                           %s- disconnect a secure user if is connected\n", KBLU, KWHT);
			printf("%s/QUIT_ROOM_SEC %sroom_name                  %s- disconnect a connected user from the room <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/JOIN_ROOM_SEC %sroom_name                  %s- to join a secure room with the name <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/DEL_ACCOUNT_SEC %sroom_name                %s- to delete an secure account named <room_name>\n", KBLU, KCYN, KWHT);
			printf("%s/MP_SEC %suser message                      %s- send a private message <message> to the user <user>\n", KBLU, KCYN, KWHT);
			printf("%s/MESSAGE %suser/room message                %s- send the message <message> to the user <user> or the room <room>\n", KBLU, KCYN, KWHT);
			printf("%s/ACCEPT_JOIN_ROOM_SEC %sroom user           %s- authorize the user <user> to join the room <room>\n", KBLU, KCYN, KWHT);
			printf("%s/REFUSE_JOIN_ROOM_SEC %sroom user           %s- refuse the user <user> to join the room <room>\n", KBLU, KCYN, KWHT);
			printf("%s/EJECT_FROM_ROOM_SEC %sroom user            %s- revoque the user <user> from the room <room>\n\n", KBLU, KCYN, KWHT);
			
			printf("----------------------------------------------------------------------------------------------------\n");
			printf("--------------- %sNB: all commands in classic user can be executed by a secure user %s------------------\n", KRED, KWHT);
			printf("----------------------------------------------------------------------------------------------------\n\n");
			printf("", KRED, KWHT);
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

