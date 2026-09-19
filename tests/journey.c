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
      if (!game_passable(g, g->map, nx, ny) || game_npc_at(g, nx, ny) >= 0 ||
          tile_def(game_tile(g, g->map, nx, ny))->guarded)
        continue;
      int n = ny * m->width + nx;
      if (prev[n] >= 0)
        continue;
      if (tile_def(game_tile(g, g->map, nx, ny))->transition && n != end)
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
    if (game_passable(g, g->map, sx, sy) && game_npc_at(g, sx, sy) < 0 &&
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
  int x, y;
  game_npc_pos(g, n, &x, &y); /* the foreman moves while he follows */
  return g->map == npcs[n].map && use(g, x, y) && g->npc == (int)n;
}
static void close_dialogue(Game *g) {
  for (int i = 0; i < DIALOGUE_PAGES + 1 && g->state == GAME_DIALOGUE; i++)
    game_action(g, ACT_CONFIRM);
}
static void see(Game *g, JourneyObserver o, void *c, const char *label);
/* The night at the inn, then the grey patch by the shrine: every outcome ends
 * the same way and says nothing more about the visitor. */
static bool sleep_and_look(Game *g, JourneyObserver o, void *c, const char *label) {
  if (g->state == GAME_PROMPT) { /* Sumi asked; the night is one answer away */
    see(g, o, c, "prompt-night");
    g->selection = 0;
    game_action(g, ACT_CONFIRM);
  } else {
    if (g->map == MAP_FOREST)
      REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
    REQUIRE(walk(g, INN_X, INN_Y));
    game_action(g, ACT_CONFIRM); /* what the futon is */
    REQUIRE(g->dialogue == D_X_FUTON);
    game_action(g, ACT_CONFIRM); /* and the question it raises */
    REQUIRE(g->state == GAME_PROMPT);
    g->selection = 0;
    game_action(g, ACT_CONFIRM);
  }
  REQUIRE(g->state == GAME_DIALOGUE && g->phase == PHASE_MORNING);
  close_dialogue(g);
  REQUIRE(walk(g, 24, 15)); /* the wood yard shows what the night brought */
  see(g, o, c, label);
  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(approach(g, 37, 19));
  game_action(g, ACT_CONFIRM);
  REQUIRE(game_knows(g, OBS_GREY_TRACE));
  close_dialogue(g);
  REQUIRE(approach(g, 38, 19));
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->dialogue == D_X_INSCRIPTION_LATE); /* the visitor, then the ending */
  close_dialogue(g);
  REQUIRE(g->dialogue == D_SCENE_TEASER && game_knows(g, OBS_TEASED));
  close_dialogue(g);
  return true;
}
static void see(Game *g, JourneyObserver o, void *c, const char *label) {
  if (o)
    o(g, label, c);
}
static bool use_item(Game *g, ItemId item) {
  ItemId owned[ITEM_COUNT];
  int count = game_owned_items(g, owned);
  if (count == 0)
    return false;
  game_action(g, ACT_INVENTORY);
  for (int i = 0; i < count && owned[g->selection] != item; i++)
    game_action(g, ACT_DOWN);
  if (g->state != GAME_INVENTORY || owned[g->selection] != item)
    return false;
  game_action(g, ACT_CONFIRM);
  return g->state == GAME_DIALOGUE;
}
static bool explore(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(g->map == MAP_FOREST && g->obs == 0 && g->note_count == 0);
  REQUIRE(g->state == GAME_DIALOGUE && g->npc == SPEAKER_SCENE);
  see(g, observer, context, "01-arrival");
  close_dialogue(g);
  REQUIRE(g->state == GAME_EXPLORATION);
  REQUIRE(use(g, 5, 20));
  REQUIRE(game_knows(g, OBS_FOX_WOUNDED));
  see(g, observer, context, "02-fox");
  close_dialogue(g);

  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(game_knows(g, OBS_ASKED_BY_SUMI));
  see(g, observer, context, "03-sumi");
  close_dialogue(g);
  REQUIRE(use(g, 4, 6));
  REQUIRE(game_knows(g, OBS_HOUSE_MARK) && g->dialogue == D_X_HOUSE_MARK);
  close_dialogue(g);
  /* The village says what its houses are, before anyone explains them. */
  REQUIRE(use(g, 5, 16));
  REQUIRE(g->dialogue == D_X_INN_SIGN);
  close_dialogue(g);
  REQUIRE(walk(g, 6, 15));
  REQUIRE(g->place && g->place_ticks > 0);
  see(g, observer, context, "03b-inn");
  REQUIRE(talk_to(g, NPC_KENTA));
  REQUIRE(game_knows(g, OBS_KENTA_STARE));
  close_dialogue(g);
  REQUIRE(talk_to(g, NPC_MIO));
  REQUIRE(g->dialogue == D_MIO_HERB && g->player.inventory.quantities[ITEM_HERB] == 1);
  close_dialogue(g);

  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(g->map == MAP_FOREST);
  REQUIRE(approach(g, 5, 20));
  REQUIRE(use_item(g, ITEM_HERB));
  REQUIRE(game_knows(g, OBS_FOX_TENDED) &&
          g->player.inventory.quantities[ITEM_HERB] == 0);
  see(g, observer, context, "04-tend-fox");
  close_dialogue(g);
  REQUIRE(walk(g, 8, 19)); /* tracks are passable: examined underfoot */
  game_action(g, ACT_CONFIRM);
  REQUIRE(game_knows(g, OBS_TRACKS));
  close_dialogue(g);
  see(g, observer, context, "05-tracks");

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
  close_dialogue(g);
  REQUIRE(use_item(g, ITEM_SHARDS));
  REQUIRE(game_knows(g, OBS_BOWL_MARK) && g->dialogue == D_I_BOWL_MARK_MATCH);
  see(g, observer, context, "06-bowl-mark");
  close_dialogue(g);

  REQUIRE(use(g, 24, 16));
  REQUIRE(game_knows(g, OBS_STONE_DRAGGED));
  close_dialogue(g);
  REQUIRE(walk(g, 24, 12)); /* the hollow is passable: examined underfoot */
  REQUIRE(g->dy != 1);      /* facing the drag marks would examine those first */
  game_action(g, ACT_CONFIRM);
  REQUIRE(game_knows(g, OBS_STONE_HOLLOW));
  close_dialogue(g);
  REQUIRE(walk(g, 24, 12));
  game_action(g, ACT_UP); /* into the grove: the kami rises */
  REQUIRE(g->state == GAME_ENCOUNTER && g->mood == MOOD_ANGRY);
  see(g, observer, context, "07-encounter");
  int options[ENCOUNTER_OPTION_LIMIT];
  int count = game_encounter_options(g, options);
  REQUIRE(count == 4); /* waiting, offering the shards, attacking, retreating */
  for (int i = 0; i < count; i++)
    if (encounter_options[options[i]].action == ENC_WAIT)
      g->selection = i;
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->mood == MOOD_WARY && g->state == GAME_ENCOUNTER);
  for (int i = 0; i < count; i++)
    if (encounter_options[options[i]].action == ENC_OFFER)
      g->selection = i;
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->mood == MOOD_ANGRY && game_knows(g, OBS_KAMI_ANGERED));
  see(g, observer, context, "08-shards-offered");
  for (int i = 0; i < count; i++)
    if (encounter_options[options[i]].action == ENC_RETREAT)
      g->selection = i;
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->state == GAME_EXPLORATION && g->y == 13);
  REQUIRE(use(g, 12, 12));
  REQUIRE(game_knows(g, OBS_CLAW_MARKS_EDGE));
  close_dialogue(g);
  REQUIRE(use(g, 16, 11));
  REQUIRE(game_knows(g, OBS_FRESH_STUMPS));
  close_dialogue(g);
  REQUIRE(use(g, 22, 11));
  REQUIRE(game_knows(g, OBS_BROKEN_ROPE));
  close_dialogue(g);

  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(game_knows(g, OBS_BOWL_OWNER) && g->dialogue == D_SUMI_OWNER);
  see(g, observer, context, "09-owner");
  close_dialogue(g);
  REQUIRE(talk_to(g, NPC_ORIHA));
  REQUIRE(g->dialogue == D_ORIHA_MEND);
  close_dialogue(g); /* her offer leads straight into the workshop */
  REQUIRE(g->state == GAME_MEND);
  game_action(g, ACT_CANCEL); /* the exploration journey leaves it for later */
  REQUIRE(g->state == GAME_EXPLORATION && g->mend_placed == 0);
  REQUIRE(talk_to(g, NPC_MIO));
  REQUIRE(g->dialogue == D_MIO_THANKS);
  close_dialogue(g);

  return true;
}
static bool pick(Game *g, EncounterAction action) {
  int options[ENCOUNTER_OPTION_LIMIT];
  int count = game_encounter_options(g, options);
  for (int i = 0; i < count; i++)
    if (encounter_options[options[i]].action == action) {
      g->selection = i;
      game_action(g, ACT_CONFIRM);
      return true;
    }
  return false;
}
bool journey(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(explore(g, observer, context));
  game_action(g, ACT_CANCEL);
  REQUIRE(g->state == GAME_NOTEBOOK && g->note_count == 19);
  see(g, observer, context, "10-notebook");
  game_action(g, ACT_UP);
  REQUIRE(g->scroll == g->note_count - NOTES_PER_PAGE - 1);
  game_action(g, ACT_CANCEL);
  REQUIRE(g->state == GAME_EXPLORATION);
  game_action(g, ACT_DEBUG);
  see(g, observer, context, "11-debug");
  game_action(g, ACT_DEBUG);
  return true;
}

/* Outcome "Bekämpfen": explore, then fight the kami to the end. */
bool journey_fight(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(explore(g, observer, context));
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(walk(g, 24, 12));
  game_action(g, ACT_UP);
  REQUIRE(g->state == GAME_ENCOUNTER);
  for (int round = 0; round < 20 && g->outcome == OUT_NONE; round++) {
    if (g->state == GAME_DIALOGUE)
      close_dialogue(g);
    if (g->state != GAME_ENCOUNTER) { /* beaten and sent home: walk back */
      REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
      REQUIRE(walk(g, 24, 12));
      game_action(g, ACT_UP);
    }
    REQUIRE(pick(g, g->player.hp <= 8 && g->player.inventory.quantities[ITEM_HERB]
                        ? ENC_HEAL
                        : ENC_ATTACK));
  }
  REQUIRE(g->outcome == OUT_FIGHT && g->state == GAME_DIALOGUE);
  see(g, observer, context, "12-fight-won");
  close_dialogue(g);
  game_action(g, ACT_UP);
  REQUIRE(g->y == 11); /* the grove is open */
  see(g, observer, context, "13-grove");
  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(g->dialogue == D_SUMI_FOUGHT);
  see(g, observer, context, "14-sumi-after");
  close_dialogue(g);
  REQUIRE(sleep_and_look(g, observer, context, "15-morning-fight"));
  return true;
}

/* Outcome "Alte Grenze wiederherstellen": push the moved stone back home. */
bool journey_boundary(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(explore(g, observer, context));
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(game_can_push(g)); /* both observations came from exploring */
  REQUIRE(walk(g, STONE_START_X, STONE_START_Y + 1));
  see(g, observer, context, "15-stone");
  for (int step = 0; step < 6 && g->outcome == OUT_NONE; step++)
    game_action(g, ACT_UP);
  REQUIRE(g->stone_x == STONE_HOLLOW_X && g->stone_y == STONE_HOLLOW_Y);
  REQUIRE(g->outcome == OUT_BOUNDARY && g->state == GAME_DIALOGUE);
  see(g, observer, context, "16-boundary");
  close_dialogue(g);
  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(g->dialogue == D_SUMI_BOUNDARY);
  see(g, observer, context, "17-sumi-boundary");
  close_dialogue(g);
  REQUIRE(g->state == GAME_PROMPT);
  g->selection = 1; /* stay a while: the night must remain available */
  game_action(g, ACT_CONFIRM);
  REQUIRE(g->state == GAME_EXPLORATION && g->phase == PHASE_BEFORE);
  REQUIRE(sleep_and_look(g, observer, context, "18-morning-boundary"));
  return true;
}

/* Kintsugi (Stage 3D): mend the bowl, let it dry, calm the kami with it. */
bool journey_mend(Game *g, JourneyObserver observer, void *context) {
  REQUIRE(explore(g, observer, context));
  REQUIRE(g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_ORIHA));
  close_dialogue(g);
  REQUIRE(g->state == GAME_MEND);
  see(g, observer, context, "18-mend");
  for (int placed = 0; placed < MEND_PIECES; placed++) {
    int pieces[MEND_PIECES];
    int count = game_mend_pieces(g, pieces);
    for (int i = 0; i < count; i++)
      if (pieces[i] == placed)
        g->selection = i;
    game_action(g, ACT_CONFIRM);
    REQUIRE(g->mend_placed == placed + 1);
  }
  REQUIRE(game_knows(g, OBS_BOWL_DRYING));
  see(g, observer, context, "19-mended");
  close_dialogue(g);
  /* Into the forest and back: the lacquer has dried by then. */
  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(game_knows(g, OBS_BOWL_READY));
  REQUIRE(talk_to(g, NPC_ORIHA));
  REQUIRE(g->dialogue == D_ORIHA_READY && g->player.inventory.quantities[ITEM_BOWL] == 1);
  see(g, observer, context, "20-bowl");
  close_dialogue(g);
  REQUIRE(walk(g, 16, 0) || g->map == MAP_FOREST);
  REQUIRE(walk(g, 24, 12));
  game_action(g, ACT_UP);
  REQUIRE(g->state == GAME_ENCOUNTER);
  REQUIRE(game_encounter_offer(g) == ITEM_BOWL);
  REQUIRE(pick(g, ENC_OFFER));
  REQUIRE(g->mood == MOOD_CALM && game_knows(g, OBS_KAMI_CALMED));
  see(g, observer, context, "21-bowl-offered");
  REQUIRE(pick(g, ENC_RETREAT));
  REQUIRE(g->state == GAME_EXPLORATION);

  /* Stage 3E: Daigo stakes out the new boundary along the tracks. */
  REQUIRE(talk_to(g, NPC_DAIGO));
  REQUIRE(g->dialogue == D_DAIGO_OFFER);
  see(g, observer, context, "22-daigo-offer");
  close_dialogue(g);
  REQUIRE(g->daigo_follows);
  for (int i = 0; i < STAKE_COUNT; i++) {
    REQUIRE(walk(g, stakes[i].x, stakes[i].y));
    game_action(g, ACT_CONFIRM);
    REQUIRE(g->state == GAME_DIALOGUE);
    close_dialogue(g);
  }
  REQUIRE(g->outcome == OUT_MEND && !g->daigo_follows);
  see(g, observer, context, "23-new-boundary");
  REQUIRE(walk(g, 24, 39) || g->map == MAP_VILLAGE);
  REQUIRE(talk_to(g, NPC_SUMI));
  REQUIRE(g->dialogue == D_SUMI_MEND);
  see(g, observer, context, "24-sumi-mend");
  close_dialogue(g);
  REQUIRE(sleep_and_look(g, observer, context, "25-morning-mend"));
  return true;
}
