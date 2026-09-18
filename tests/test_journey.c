#include "journey.h"
#include <stdio.h>
int main(int argc, char **argv) {
  Game g;
  if (argc != 2)
    return 1;
  if (!game_init(&g, argv[1]) || !journey(&g, NULL, NULL))
    return 1;
  if (!game_init(&g, argv[1]) || !journey_fight(&g, NULL, NULL))
    return 1;
  puts("Exploration and the fight outcome pass using only gameplay actions.");
  return 0;
}
