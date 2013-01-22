#ifndef COMMON
#define COMMON

#define MAX_NAME_SIZE 64
#define MAX_MESS_SIZE 512
#define MAX_ROOM_NAME_SIZE 64

#define CREATE_ROOM 0xA1
#define QUIT_ROOM 	0XA2
#define JOIN_ROOM	0xA3
#define DISCONNECT	0xB1
#define CONNECT		0xC1
#define MESSAGE 	0xD1

typedef struct {
	int code;
	char[MAX_NAME_SIZE] name;
	char[MAX_MESS_SIZE] mess;
	char[MAX_ROOM_NAME_SIZE] room;
} message;

#endif