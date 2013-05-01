#include "common.h"
#include "user_manager.h"
#include "room_manager.h"


int set_keyiv_in_room (char *room_name, key_iv keyiv);

key_iv get_keyiv_in_room (char *room_name);

int add_room_client (char *room_name);
