#include "libclientsec.h"

#define CHATADDR "localhost"
#define CHATPORT 10000

#define SECADDR  "localhost"
#define SECPORT  11000

int main (int argc, char *argv[]) {
    connect_with_authentication (CHATADDR, CHATPORT, "toto", SECADDR, SECPORT);
}
