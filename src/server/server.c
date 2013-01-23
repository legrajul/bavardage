#include "lib_server.h"

room *room;
user *users;
SocketTCP *listen;
message *buf;
char *addr;
int port;
int client;
pthread_t thread;



int setup_socket() {

/*Traitement d'une connexion lancée dans un thread*/
     void *handle_connexion(void *param) {
          message *mess = (message *) param;
          users->name = mess->name;
          set_ip(listen->distant.ip);
          set_port(listen->distant.port);
          if (pthread_join(thread, NULL)) {
             perror("pthread_join");
          }

      }

/*Création de la socket serveur*/
           if ((listen = creerSocketEcouteTCP(addr, port)) == NULL) {
              perror("creerSocketEcouteTCP");
              return -1;
           }

/*Attente de connexion d'un client et création de thread*/    
    while (1) {
        
        client = acceptSocketTCP(listen); //on accepte la connexion du client
        int size = readSocketTCP(listen, buf, sizeof(message)); //lecture de la socket et des données envoyées

         if (size >= 0) {
            if(pthread_create(&thread, NULL, handle_connexion, buf)) {  //Création du thread
               perror("pthread_create");
               
             }
        }
        
      closeSocketTCP(listen); //Fermeture de la socket
}




