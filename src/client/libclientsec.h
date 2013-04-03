#ifndef _LIBCLIENTSEC_H
#define _LIBCLIENTSEC_H

#define CertFile "toto_certif.pem"
#define KeyFile "toto.pem"

#include "libclient.h"

#include "../common/SocketTCP.h"

int connect_with_authentication (char *chatservaddr, int chatservport, char *login,
				 char *secservaddr, int secservport);

int disconnect_servers ();

int generate_private_key ();

int generate_certificate_request (char *common_name, char *locality, char *country, 
				  char* organization, char *email_address);

int get_certificate (char *pkiaddr);

int send_message_sec (const char *mess, char **error_mess);

#endif
