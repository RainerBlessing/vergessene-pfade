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
  if (!game_init(&g, argv[1]) || !journey_boundary(&g, NULL, NULL))
    return 1;
  puts("Exploration, the fight and the boundary pass using only gameplay actions.");
  return 0;
}
