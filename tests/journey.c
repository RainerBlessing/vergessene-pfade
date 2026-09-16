#include "journey.h"
#include <stdio.h>
#define REQUIRE(x)                                                                       \
  do {                                                                                   \
    if (!(x)) {                                                                          \
      fprintf(stderr, "Journey failed at %d: %s\n", __LINE__, #x);                       \
      return false;                                                                      \
    }                                                                                    \
  } while (0)
static const int dxs[4] = {0, 0, -1, 1}, dys[4] = {-1, 1, 0, 0};
static const Action moves[4] = {ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT};
/* Breadth-first navigation issues only real movement actions. It never edits
 * state. Returns false without side effects when the target is unreachable. */
static bool walk(Game *g, int tx, int ty) {
  const Map *m = &g->maps[g->map];
  int prev[MAP_LIMIT * MAP_LIMIT], queue[MAP_LIMIT * MAP_LIMIT], head = 0, tail = 0;
  for (int i = 0; i < MAP_LIMIT * MAP_LIMIT; i++)
    prev[i] = -1;
  int start = g->y * m->width + g->x, end = ty * m->width + tx;
  queue[tail++] = start;
  prev[start] = start;
  while (head < tail && prev[end] < 0) {
    int at = queue[head++], x = at % m->width, y = at / m->width;
    for (int i = 0; i < 4; i++) {
      int nx = x + dxs[i], ny = y + dys[i];
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
  if (prev[end] < 0)
    return false;
  int path[MAP_LIMIT * MAP_LIMIT], count = 0;
  for (int at = end; at != start; at = prev[at])
    path[count++] = at;
  while (count) {
    int to = path[--count], from = prev[to];
    int vx = to % m->width - from % m->width, vy = to / m->width - from / m->width;
    for (int i = 0; i < 4; i++)
      if (dxs[i] == vx && dys[i] == vy)
        game_action(g, moves[i]);
  }
  return g->x == tx && g->y == ty;
}
/* Walk next to a blocking target (NPC or tile) and face it. */
static bool approach(Game *g, int tx, int ty) {
  for (int i = 0; i < 4; i++) {
    int sx = tx - dxs[i], sy = ty - dys[i];
    if (map_passable(&g->maps[g->map], sx, sy) && game_npc_at(g, sx, sy) < 0 &&
        walk(g, sx, sy)) {
      game_action(g, moves[i]);
      return g->x == sx && g->y == sy && g->dx == dxs[i] && g->dy == dys[i];
    }
  }
  return false;
}
static bool use(Game *g, int tx, int ty) {
  if (!approach(g, tx, ty))
    return false;
  game_action(g, ACT_CONFIRM);
  return g->state == GAME_DIALOGUE;
}
static bool talk_to(Game *g, NpcId n) {
  return g->map == npcs[n].map && use(g, npcs[n].x, npcs[n].y) && g->npc == (int)n;
}
static void close_dialogue(Game *g) {
  for (int i = 0; i < DIALOGUE_PAGES + 1 && g->state == GAME_DIALOGUE; i++)
    game_action(g, ACT_CONFIRM);
}
static void see(Game *g, JourneyObserver o, void *c, const char *label) {
  if (o)
    o(g, label, c);
}
bool journey(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(g->map == MAP_VILLAGE && g->obs == 0 && g->note_count == 0);
  see(g, observer, context, "01-village");
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(game_knows(g, OBS_ASKED_BY_SUMI));
  see(g, observer, context, "02-sumi");
  close_dialogue(g);
  REQUIRE(use(g, 4, 6));
  REQUIRE(game_knows(g, OBS_HOUSE_MARK) && g->dialogue == D_X_HOUSE_MARK);
  see(g, observer, context, "03-house-mark");
  close_dialogue(g);
  REQUIRE(talk_to(g, NPC_KENTA));
  REQUIRE(game_knows(g, OBS_KENTA_STARE));
  close_dialogue(g);

  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(g->map == MAP_FOREST);
  see(g, observer, context, "04-forest");
  REQUIRE(talk_to(g, NPC_DAIGO));
  close_dialogue(g);
  REQUIRE(use(g, 12, 30));
  REQUIRE(game_knows(g, OBS_LEDGER_DEBT));
  close_dialogue(g);
  REQUIRE(use(g, 38, 19));
  REQUIRE(game_knows(g, OBS_SHRINE_INSCRIPTION));
  close_dialogue(g);
  REQUIRE(use(g, 38, 20));
  REQUIRE(game_knows(g, OBS_BOWL_SHARDS) &&
          g->player.inventory.quantities[ITEM_SHARDS] == 1);
  see(g, observer, context, "05-shards");
  close_dialogue(g);
  game_action(g, ACT_INVENTORY);
  REQUIRE(g->state == GAME_INVENTORY);
  see(g, observer, context, "06-bag");
  game_action(g, ACT_CONFIRM);
  REQUIRE(game_knows(g, OBS_BOWL_MARK) && g->dialogue == D_I_BOWL_MARK_MATCH);
  close_dialogue(g);

  REQUIRE(use(g, 24, 16));
  REQUIRE(game_knows(g, OBS_STONE_DRAGGED));
  close_dialogue(g);
  REQUIRE(walk(g, 24, 12)); /* the hollow is passable: examined underfoot */
  REQUIRE(g->dy != 1);      /* facing the drag marks would examine those first */
  game_action(g, ACT_CONFIRM);
  REQUIRE(game_knows(g, OBS_STONE_HOLLOW));
  see(g, observer, context, "07-hollow");
  close_dialogue(g);
  REQUIRE(use(g, 12, 12));
  REQUIRE(game_knows(g, OBS_CLAW_MARKS_EDGE));
  close_dialogue(g);
  REQUIRE(use(g, 16, 11));
  REQUIRE(game_knows(g, OBS_FRESH_STUMPS));
  close_dialogue(g);
  REQUIRE(use(g, 24, 5));
  REQUIRE(game_knows(g, OBS_BROKEN_ROPE));
  close_dialogue(g);

  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(game_knows(g, OBS_BOWL_OWNER) && g->dialogue == D_SUMI_OWNER);
  see(g, observer, context, "08-owner");
  close_dialogue(g);
  REQUIRE(talk_to(g, NPC_ORIHA));
  REQUIRE(g->dialogue == D_ORIHA_OWNER);
  close_dialogue(g);

  game_action(g, ACT_CANCEL);
  REQUIRE(g->state == GAME_NOTEBOOK && g->note_count == 13);
  see(g, observer, context, "09-notebook");
  game_action(g, ACT_UP);
  REQUIRE(g->scroll == g->note_count - NOTES_PER_PAGE - 1);
  game_action(g, ACT_CANCEL);
  REQUIRE(g->state == GAME_EXPLORATION);
  game_action(g, ACT_DEBUG);
  see(g, observer, context, "10-debug");
  game_action(g, ACT_DEBUG);
  return true;
}
