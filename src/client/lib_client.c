#include "lib_client.h"
#include "../common/common.h"
#include "../common/SocketTCP.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>

SocketTCP *client_sock;
message *msg;
char *login, **tab_string;
int status = NOT_CONNECTED;

void my_sigaction(int s) {
    switch (s) {
    case SIGINT:
	disconnect ();
	break;
    default:
	break;
    }
}

char *str_sub (const char *s, unsigned int start, unsigned int end) {
    char *new_s = NULL;
    if (s != NULL && start < end) {
        new_s = malloc (sizeof (*new_s) * (end - start + 2));
        if (new_s != NULL) {
            int i;
            for (i = start; i < end; i++) {
                new_s[i-start] = s[i];
            }
            new_s[i-start] = '\0';
        } else {
            fprintf (stderr, "Out of memory\n");
            exit (EXIT_FAILURE);
        }
    }
    return new_s;
}

int extract_code (const char *str) {
    char *command = NULL;
    command = str_sub(str, 1, strlen(str));

    if (strcmp(command, "CREATE_ROOM") == 0) {
        return CREATE_ROOM;
    } else if (strcmp(command, "DELETE_ROOM") == 0) {
	return DELETE_ROOM;
    } else if (strcmp(command, "DISCONNECT") == 0) {
        return DISCONNECT;
    } else if (strcmp(command, "CONNECT") == 0) {
        return CONNECT;
    } else if (strcmp(command, "QUIT_ROOM") == 0) {
        return QUIT_ROOM;
    } else if (strcmp(command, "JOIN_ROOM") == 0) {
        return JOIN_ROOM;
    } else if (strcmp(command, "MESSAGE") == 0) {
        return MESSAGE;
    } else if (strcmp(command, "MP") == 0) {
        return MP;
    }

    return -1;
}

int connect_socket (const char *addr, const int port) {
    int co;
    if ((client_sock = creerSocketTCP()) == NULL) {
        perror("creerSocketTCP");
        return -1;
    }
    printf ("Please wait for connecting the server...\n");
    if ((co = connectSocketTCP(client_sock, addr, port)) == -1 ) {
    	perror("connectSocketTCP");
        return -1;
    }
    
    printf("You can now send commands and messages\n");
    (void) signal (SIGINT, my_sigaction);

    return 0;
}

int receive_message (message *m) {
    while (readSocketTCP(client_sock, (char *) m, sizeof (message)) == 0);
    if (m == NULL) {
	perror ("receive_message");
	return -1;
    } else {
	if (msg->code == CONNECT && m->code == OK && status == NOT_CONNECTED) {
	    status = CONNECTED;
	}
	return 0;
    }
}


int len (char **tab) {
    int n = 0;
    if (tab == NULL)
	return n;
    else {
	int l=0;
	while (tab[l] != NULL) {
	    n++;
	    l += sizeof(char *)/sizeof(tab[l]);
	}
	return n;
    }
}

int send_command () {
    if (msg == NULL) {
	msg = (message*) malloc(sizeof(message));
    }
    if (login != NULL)
	strcpy(msg->sender, login);
    writeSocketTCP(client_sock, (char *) msg, sizeof(message));
    
    return 0;
}

char **create_table_param(const char *string) {
    char **res = (char **) malloc(sizeof(char*) * MAX_MESS_SIZE);
    int i=0;
    if (string != NULL) {
	char *tok = strtok(strdup (string), " ");
	while (tok != NULL) {
	    res[i] = tok;
	    tok = strtok(NULL, " ");
	    i++;
	}
    }
    return res;
}

int send_message (const char *mess) {
    int code;
    char buffer[strlen(mess)];
    strcpy(buffer, mess);
    msg = (message*) malloc(sizeof(message));
    
    if (mess[0] == '/') {	
        code = extract_code(strtok(strdup(buffer), " "));
        if (code == -1) {
            perror("extract_code");
            return -1;
        }
        msg->code = code;
	char *tmp, buff[MAX_MESS_SIZE] = "";
	int i;
	switch (code) {
	case CONNECT:	// Cas d'une demande de connexion
	    if (status == NOT_CONNECTED) {
		tmp = strtok (NULL, " ");
		if (tmp != NULL) {
		    login = strdup (tmp);
		}
		if (login == NULL) {
		    printf ("login null\n");
		}
		strcpy(msg->sender, login);
		send_command ();
	    }         
	    break;
	    
	case DISCONNECT:	// Cas d'une demande de déconnexion
	    strcpy(msg->sender, login);
	    disconnect();
	    break;
	    
	case CREATE_ROOM:	// Cas d'une demande de création de Salon
	    tmp = strtok (NULL, " ");
	    if (tmp != NULL) {
		strcpy (msg->content, tmp);
	    }
	    send_command ();
	    break;
	case DELETE_ROOM:
	    tmp = strtok (NULL, " ");
	    if (tmp != NULL) {
		strcpy (msg->content, tmp);
	    }
	    send_command ();
	    break;
	case QUIT_ROOM:		// Cas d'une demande pour quitter une room
	    tmp = strtok (NULL, " ");
	    if (tmp!= NULL) {
		strcpy (msg->content, tmp);
	    }
	    strcpy(msg->sender, login);
	    send_command ();
		    
	    break;
		    
	case JOIN_ROOM:
	    tmp = strtok (NULL, " ");
	    if (tmp != NULL) {
		strcpy (msg->content, tmp);
	    }
	    send_command ();
	    break;
		    
		    
	case MESSAGE:  // Cas d'envoi de message
	    tab_string = create_table_param(buffer);
	    strcpy(msg->receiver, tab_string[1]);
	    for (i=2; i<len(tab_string); i++) {
		strcat(buff, tab_string[i]);
		strcat(buff, " ");
	    }
		    
	    strcpy(msg->content, buff);
	    send_command ();
	    break;
		    
	case MP:  // Cas d'envoi de message prive
	    tab_string = create_table_param(buffer);
	    strcpy(msg->receiver, tab_string[1]);
	    for (i=2; i<len(tab_string); i++) {
		strcat(buff, tab_string[i]);
		strcat(buff, " ");
	    }
		    
	    strcpy(msg->content, buff);
	    send_command ();
	    break;
	}
    }
    return 0;
}

int disconnect() {
    msg->code = DISCONNECT;
    send_command();
    return 0;
}

int get_last_request_code () {
    return msg->code;
}

int is_connected () {
    if (status == CONNECTED) {
	return 1;
    } else {
	return 0;
    }
}
