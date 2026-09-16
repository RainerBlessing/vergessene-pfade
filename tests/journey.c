#include "journey.h"
#include <stdio.h>
#define REQUIRE(x)                                                             \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "Journey failed at %d: %s\n", __LINE__, #x);             \
      return false;                                                            \
    }                                                                          \
  } while (0)
/* Breadth-first navigation issues only real movement actions. It never edits
 * state. */
static bool walk(Game *g, int tx, int ty) {
  const Map *m = &g->maps[g->map];
  int prev[MAP_LIMIT * MAP_LIMIT], queue[MAP_LIMIT * MAP_LIMIT], head = 0,
                                                                 tail = 0;
  const int dx[4] = {0, 0, -1, 1}, dy[4] = {-1, 1, 0, 0};
  const Action a[4] = {ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT};
  for (int i = 0; i < MAP_LIMIT * MAP_LIMIT; i++)
    prev[i] = -1;
  int start = g->y * m->width + g->x, end = ty * m->width + tx;
  queue[tail++] = start;
  prev[start] = start;
  while (head < tail && prev[end] < 0) {
    int at = queue[head++], x = at % m->width, y = at / m->width;
    for (int i = 0; i < 4; i++) {
      int nx = x + dx[i], ny = y + dy[i];
      if (!map_passable(m, nx, ny) || game_npc_at(g, nx, ny) >= 0)
        continue;
      int n = ny * m->width + nx;
      if (prev[n] >= 0)
        continue;
      if (tile_def(map_at(m, nx, ny))->transition && n != end)
        continue;
      prev[n] = at;
      queue[tail++] = n;
    }
  }
  REQUIRE(prev[end] >= 0);
  int path[MAP_LIMIT * MAP_LIMIT], count = 0;
  for (int at = end; at != start; at = prev[at])
    path[count++] = at;
  while (count) {
    int to = path[--count], from = prev[to];
    int vx = to % m->width - from % m->width,
        vy = to / m->width - from / m->width;
    for (int i = 0; i < 4; i++)
      if (dx[i] == vx && dy[i] == vy)
        game_action(g, a[i]);
  }
  return true;
}
static void close_dialogue(Game *g) {
  for (int i = 0; i < 4 && g->state == GAME_DIALOGUE; i++)
    game_action(g, ACT_CONFIRM);
}
static void see(Game *g, JourneyObserver o, void *c, const char *label) {
  if (o)
    o(g, label, c);
}
bool journey(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(g->quest == NOT_STARTED);
  see(g, observer, context, "01-world");
  REQUIRE(walk(g, 10, 8));
  REQUIRE(g->map == 1);
  see(g, observer, context, "02-town");
  REQUIRE(walk(g, 10, 8));
  game_action(g, ACT_UP);
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->state == GAME_DIALOGUE && g->quest == ACTIVE);
  see(g, observer, context, "03-dialogue");
  close_dialogue(g);
  REQUIRE(walk(g, 19, 20));
  game_action(g, ACT_UP);
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->npc == 3);
  close_dialogue(g);
  REQUIRE(walk(g, 8, 16));
  game_action(g, ACT_UP);
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->npc == 2);
  close_dialogue(g);
  REQUIRE(walk(g, 22, 10));
  game_action(g, ACT_UP);
  game_action(g, ACT_CONFIRM);
  close_dialogue(g);
  REQUIRE(g->state == GAME_SHOP);
  int gold = g->player.gold;
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->player.gold == gold - 6);
  see(g, observer, context, "04-shop");
  game_action(g, ACT_CANCEL);
  game_action(g, ACT_INVENTORY);
  REQUIRE(g->state == GAME_INVENTORY);
  see(g, observer, context, "05-inventory");
  game_action(g, ACT_CANCEL);
  REQUIRE(walk(g, 15, 28));
  REQUIRE(g->map == 0);
  REQUIRE(walk(g, 38, 8));
  REQUIRE(g->state == GAME_COMBAT);
  see(g, observer, context, "06-combat");
  for (int turn = 0; turn < 20 && !g->combat.won && !g->combat.lost; turn++) {
    int wanted =
        g->player.hp <= 12 && g->player.inventory.quantities[ITEM_HERB] > 0 ? 1
                                                                            : 0;
    if (g->selection != wanted)
      game_action(g, ACT_DOWN);
    game_action(g, ACT_CONFIRM);
  }
  REQUIRE(g->combat.won && g->quest == OBJECTIVE_FOUND);
  see(g, observer, context, "07-victory");
  game_action(g, ACT_CONFIRM);
  REQUIRE(walk(g, 10, 8));
  REQUIRE(g->map == 1);
  REQUIRE(walk(g, 10, 8));
  game_action(g, ACT_UP);
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->quest == COMPLETED &&
          g->player.inventory.quantities[ITEM_LENS] == 0);
  see(g, observer, context, "08-complete");
  game_action(g, ACT_CONFIRM);
  see(g, observer, context, "09-reward");
  gold = g->player.gold;
  close_dialogue(g);
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->dialogue == 6 && g->player.gold == gold);
  close_dialogue(g);
  game_action(g, ACT_CANCEL);
  REQUIRE(g->state == GAME_PAUSED);
  see(g, observer, context, "10-journal");
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->state == GAME_EXPLORATION);
  game_action(g, ACT_DEBUG);
  see(g, observer, context, "11-debug");
  game_action(g, ACT_DEBUG);
  return true;
}
