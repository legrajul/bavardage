#ifndef LIB_CLIENT
#define LIB_CLIENT

int send_message (const char* mess);

int send_commande (const int code, const char* param);

int connect (const char* addr, const int port);

int disconnect ();

int switch_to_room (const char* room_name);

int ask_to_join_room (const char* room_name);

/* struct rooms
	char** rooms
	char* current_room
	char* login 
*/

#endif