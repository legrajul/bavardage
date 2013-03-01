#ifndef COMMON
#define COMMON

#define MAX_NAME_SIZE 64
#define MAX_MESS_SIZE 512
#define MAX_ROOM_NAME_SIZE 64

#define CREATE_ROOM     1
#define DELETE_ROOM     2
#define QUIT_ROOM 	3
#define JOIN_ROOM	4
#define DISCONNECT	5
#define CONNECT		6
#define MESSAGE 	7
#define OK		8
#define KO		9
#define LOGIN_IN_USE   10
#define NOT_CONNECTED  11
#define CONNECTED      12
#define MP 15
#define ADD_USER 16
#define RM_USER 17
#define NEW_USER 18
#define ADMIN 19

typedef struct {
	int code;
	char sender[MAX_NAME_SIZE];
	char content[MAX_MESS_SIZE];
	char receiver[MAX_ROOM_NAME_SIZE];
} message;

#endif
