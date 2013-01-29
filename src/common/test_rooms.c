#include "room_manager.h"
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
  init_rooms ();
  add_room ("accueil", NULL);
  add_user_in_room ("toto", "accueil");
  remove_user_from_room ("toto", "accueil");
  return 0;
}
