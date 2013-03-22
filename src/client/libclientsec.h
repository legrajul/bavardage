#ifndef _LIBCLIENTSEC_H
#define _LIBCLIENTSEC_H

#include "libclient.h"

int connect_with_authentication (char *chatservaddr, int chatservport, char *login,
				 char *secservaddr, int secservaddr);

int disconnect_servers ();

int generate_private_key ();

int generate_certificate_request (char *common_name, char *locality, char *country, 
				  char* organization, char *email_address);

int get_certificate (char *pkiaddr);

#endif
