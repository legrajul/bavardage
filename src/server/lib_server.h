#include "../common/common.h"
#include "../common/SocketTCP.h"

typedef struct {
        char name[MAX_NAME_SIZE];
        SocketTCP s;
} user;

typedef struct  {
        char login[MAX_ROOM_NAME_SIZE];
        user admin;
        user *users;
} room;


void *handle_connexion(void *param);
/**
*   Traite une connexion lancée dans un thread 
*   Lit les données envoyées par le client et
*   Envoie une réponse au client selon la commande envoyée par le client
*	@param param Socket du client
*/

int start_listening(const char *addr, int port);
/**
*   Crée la socket serveur
*   Se met en écoute pour attendre la connexion d'un client
*   Crée un thread
*   Retourne -1 si erreur de création de socket ou de thread 
*   retourne 1 si pas d'erreur
*	@param addr adresse ip du serveur
*   @param port port du serveur
*/

void new_thread(SocketTCP *socket);
/**
*   Initie un thread  
*   Crée un thread
*   Détache le thread
*   Détruit le thread
*	@param socket socket client
*/

int clear_message (message *m);
/**
 * Vide la structure message
 * @param m structure message
 */


int create_room(char *room_name);
int set_ip(char *ip);
int set_port(int p);
