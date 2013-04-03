#include "libclientsec.h"

#define CHATADDR "localhost"
#define CHATPORT 10000

#define SECADDR  "localhost"
#define SECPORT  11000

int main (int argc, char *argv[]) {
    set_private_key_filename ("toto.pem");
    set_certif_filename ("toto_certif.pem");
    connect_with_authentication (CHATADDR, CHATPORT, argv[1], SECADDR, SECPORT);
    return 0;
}
