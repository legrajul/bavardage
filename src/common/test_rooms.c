#include "room_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
  init_rooms ();
  add_room ("accueil", NULL);
  user u;
  u = (user) malloc (sizeof (struct USER));
  strcpy (u->name, "toto");
  add_user_in_room (u, "accueil");
  u = (user) malloc (sizeof (struct USER));
  strcpy (u->name, "tata");
  add_user_in_room (u, "accueil");
  u = (user) malloc (sizeof (struct USER));
  strcpy (u->name, "titi");
  add_user_in_room (u, "accueil");

  //user_list l = get_users ("accueil");

  //for (; l != NULL; l = l->next) {
  //  printf ("user : %s\n", l->current_user->name);
  //}

  u = (user) malloc (sizeof (struct USER));
  strcpy (u->name, "toto");
  printf ("toto dans accueil ? %d\n", is_user_in_room (u, "accueil"));
  u = (user) malloc (sizeof (struct USER));
  strcpy (u->name, "tutu");
  printf ("tutu dans accueil ? %d\n", is_user_in_room (u, "accueil"));

  // remove_user_from_room ("toto", "accueil");
  return 0;
}
