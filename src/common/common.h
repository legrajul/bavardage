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
#define OK			0xF5
#define KO			0x5F

typedef struct {
	int code;
	char name[MAX_NAME_SIZE];
	char mess[MAX_MESS_SIZE];
	char room[MAX_ROOM_NAME_SIZE];
} message;

#endif