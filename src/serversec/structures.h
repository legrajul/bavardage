#include <openssl/ssl.h>
#include "../common/common.h"

/**
 * Structure représentant un utilisateur
 */
typedef struct USER_SEC {
    char name[MAX_NAME_SIZE];
    SSL *ssl;
} *user_sec;
