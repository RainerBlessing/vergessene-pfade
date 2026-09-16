#include "journey.h"
#include <stdio.h>
int main(int argc, char **argv) {
  Game g;
  if (argc != 2 || !game_init(&g, argv[1]) || !journey(&g, NULL, NULL))
    return 1;
  puts("Complete journey passed using only gameplay actions.");
  return 0;
}
