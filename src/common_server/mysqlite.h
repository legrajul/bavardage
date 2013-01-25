#ifndef _MYSQLITE_H
#define _MYSQLITE_H

  #define QUERY_SIZE 524

  // commun au serveurs
  int connect_server_database();
  int add_user(char *login);
  int delete_user(char *login);
  int check_user(char *login);
  void close_server_database();

#endif
