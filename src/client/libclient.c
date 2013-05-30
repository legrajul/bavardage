#include "libclient.h"
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


char *str_sub (const char *s, unsigned int start, unsigned int end) {
	char *new_s = NULL;
	if (s != NULL && start < end) {
		new_s = malloc (sizeof(*new_s) * (end - start + 2));
		if (new_s != NULL) {
			int i;
			for (i = start; i < end; i++) {
				new_s[i - start] = s[i];
			}
			new_s[i - start] = '\0';
		} else {
			fprintf (stderr, "Out of memory\n");
			exit (EXIT_FAILURE);
		}
	}
	return new_s;
}

int extract_code (const char *str) {
	char *command = NULL;
	command = str + 1;

	if (strcmp (command, "CREATE_ROOM") == 0) {
		return CREATE_ROOM;
	} else if (strcmp (command, "DELETE_ROOM") == 0) {
		return DELETE_ROOM;
	} else if (strcmp (command, "DISCONNECT") == 0) {
		return DISCONNECT;
	} else if (strcmp (command, "CONNECT") == 0) {
		return CONNECT;
	} else if (strcmp (command, "QUIT_ROOM") == 0) {
		return QUIT_ROOM;
	} else if (strcmp (command, "JOIN_ROOM") == 0) {
		return JOIN_ROOM;
	} else if (strcmp (command, "MESSAGE") == 0) {
		return MESSAGE;
	} else if (strcmp (command, "MP") == 0) {
		return MP;
	}

	return -1;
}

int connect_socket (const char *addr, const int port) {
	int co;
	if ((client_sock = creerSocketTCP ()) == NULL) {
		return -1;
	}
	printf ("Please wait for connecting the server...\n");
	if ((co = connectSocketTCP (client_sock, addr, port)) == -1) {
		return -1;
	}

	printf ("You can now send commands and messages\n");

	return 0;
}

int receive_message (message *m) {
   // printf("receive_message libclient\n");
	int ret = readSocketTCP (client_sock, (char *) m, sizeof(message));
	if (ret == 0) {
		return -1;
	} else {
		return 0;
	}
}

int len (char **tab) {
	int n = 0;
	if (tab == NULL)
		return n;
	else {
		while (tab[n] != NULL) {
			n++;
		}
		return n;
	}
}

int send_command () {
	if (client_sock == NULL) {
		return -1;
	}
	if (msg == NULL) {
		msg = (message*) malloc (sizeof(message));
	}
	if (login != NULL)
		strcpy (msg->sender, login);
	else
		return -1;

    printf ("msg->code = %d, msg->sender = %s, msg->content = %s, msg->receiver = %s\n", msg->code, msg->sender, msg->content, msg->receiver);
	if (writeSocketTCP (client_sock, (char *) msg, sizeof(message)) < 0) {
		return (1);
	}

	return 0;
}

char **create_table_param (const char *string) {
	char **res = (char **) malloc (sizeof(char*) * 100);
	int i = 0;
	if (string != NULL) {
		char *tok = strtok (strdup (string), " ");
		while (tok != NULL) {
			res[i] = tok;
			tok = strtok (NULL, " ");
			i++;
		}
		int j;
		for (j = i; j < 100; j++) {
			res[j] = NULL;
		}
	}
	return res;
}

int send_message (const char *mess, char **error_mess) {
	int code;
	char buffer[20 + MAX_NAME_SIZE + MAX_MESS_SIZE] = "";
	strcpy (buffer, mess);
	buffer[strlen (buffer)] = '\0';

	msg = (message*) malloc (sizeof(message));

	if (mess[0] == '/') {
		code = extract_code (strtok (strdup (buffer), " "));
		if (code == -1) {
			return -2;
		}
		msg->code = code;
		char *tmp, buff[MAX_MESS_SIZE] = "";
		int i;

		switch (code) {
		case CONNECT:   // Cas d'une demande de connexion
			tmp = strtok (NULL, " ");
			if (tmp != NULL) {
				login = strdup (tmp);
			}
			if (login == NULL) {
				printf ("login null\n");
				return -1;
			} else {
				strcpy (msg->sender, login);
				return send_command ();
			}
			break;

		case DISCONNECT:        // Cas d'une demande de déconnexion
			strcpy (msg->sender, login);
			disconnect ();
			break;

		case CREATE_ROOM:       // Cas d'une demande de création de Salon
			tmp = strtok (NULL, " ");
			if (tmp != NULL) {
				strcpy (msg->content, tmp);
			} else {
				*error_mess = strdup ("CREATE_ROOM a besoin d'un paramètre\n");
				return -3;
			}
			return send_command ();
			break;
		case DELETE_ROOM:
			tmp = strtok (NULL, " ");
			if (tmp != NULL) {
				strcpy (msg->content, tmp);
			} else {
				*error_mess = strdup ("DELETE_ROOM a besoin d'un paramètre\n");
				return -3;
			}
			//printf("------- send_command DELETE_ROOM libclient ----------\n");
			return send_command ();
			break;
		case QUIT_ROOM:         // Cas d'une demande pour quitter une room
			tmp = strtok (NULL, " ");
			if (tmp != NULL) {
				strcpy (msg->content, tmp);
			} else {
				*error_mess = strdup ("QUIT_ROOM a besoin d'un paramètre\n");
				return -3;
			}
			strcpy (msg->sender, login);
			return send_command ();

			break;

		case JOIN_ROOM:
			tmp = strtok (NULL, " ");
			if (tmp != NULL) {
				strcpy (msg->content, tmp);
			} else {
				*error_mess = strdup ("JOIN_ROOM a besoin d'un paramètre\n");
				return -3;
			}
			return send_command ();
			break;

		case MESSAGE:  // Cas d'envoi de message
			tab_string = create_table_param (buffer);
			if (len (tab_string) < 3) {
				*error_mess =
						strdup (
								"MESSAGE doit avoir 2 paramètres : /MESSAGE salon mon super message\n");
				return -3;
			}
			strcpy (msg->receiver, tab_string[1]);
			memcpy (msg->content, buffer + 10 + strlen (msg->receiver), MAX_MESS_SIZE);
			free (tab_string);
			return send_command ();
			break;

		case MP:  // Cas d'envoi de message prive
			tab_string = create_table_param (buffer);

			if (len (tab_string) < 3) {
				*error_mess =
						strdup (
								"MP doit avoir 2 paramètres : /MP toto mon super message privé\n");
				return -3;
			}
			strcpy (msg->receiver, tab_string[1]);
            memcpy (msg->content, buffer + 5 + strlen (msg->receiver), MAX_MESS_SIZE);
			return send_command ();
			break;
		}
	}
	return 0;
}

int disconnect () {
	if (client_sock != NULL && msg != NULL) {
		msg->code = DISCONNECT;
		send_command ();
	}
	return 0;
}

char *get_login () {
	return strdup (login);
}
