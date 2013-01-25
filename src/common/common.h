#ifndef COMMON
#define COMMON

#define MAX_NAME_SIZE 64
#define MAX_MESS_SIZE 512
#define MAX_ROOM_NAME_SIZE 64

#define CREATE_ROOM 0x01
#define QUIT_ROOM 	0X02
#define JOIN_ROOM	0x03
#define DISCONNECT	0x04
#define CONNECT		0x05
#define MESSAGE 	0x06
#define OK			0x07
#define KO			0x08
#define LOGIN_IN_USE  0x09
#define NOT_CONNECTED 0x0A
#define CONNECTED 	0x0B

typedef struct {
	int code;
	char name[MAX_NAME_SIZE];
	char mess[MAX_MESS_SIZE];
	char room[MAX_ROOM_NAME_SIZE];
} message;

#endif
