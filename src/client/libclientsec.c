#include "libclientsec.h"

char *private_key_filename;
char *certif_request_filename;
char *certif_filename;

int is_connected = 0;


int connect_with_authentication (char *chatservaddr, int chatservport, char *login,
				 char *secservaddr, int secservaddr) { 
    //TODO
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
