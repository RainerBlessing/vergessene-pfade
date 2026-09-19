#include "combat.h"
#include "game.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                         \
  do {                                                                                   \
    if (!(x)) {                                                                          \
      fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x);                               \
      return 1;                                                                          \
    }                                                                                    \
  } while (0)

/* Place the player and face a direction without walking. */
static void stand(Game *g, int map, int x, int y, int dx, int dy) {
  g->map = map;
  g->x = x;
  g->y = y;
  g->dx = dx;
  g->dy = dy;
  g->state = GAME_EXPLORATION;
}
/* Close whatever is open: a dialogue, and the question that may follow it. */
static void dismiss(Game *g) {
  for (int i = 0; i < 8 && (g->state == GAME_DIALOGUE || g->state == GAME_PROMPT); i++)
    game_action(g, ACT_CANCEL);
}
static int count_events(const Game *g, EventType type) {
  int n = 0;
  for (int i = 0; i < g->event_count; i++)
    n += g->events[i].type == type;
  return n;
}
static bool text_fits(const char *s, int max_lines) {
  int lines = 1, columns = 0;
  for (; *s; s++) {
    if (*s == '\n') {
      lines++;
      columns = 0;
    } else if (++columns > 36)
      return false;
  }
  return lines <= max_lines;
}
/* Is a tile reachable on foot from (sx,sy), not crossing map transitions? */
static bool reachable(const Game *g, int map, int sx, int sy, int tx, int ty) {
  const Map *m = &g->maps[map];
  static bool seen[MAP_LIMIT * MAP_LIMIT];
  int queue[MAP_LIMIT * MAP_LIMIT], head = 0, tail = 0;
  memset(seen, 0, sizeof seen);
  queue[tail++] = sy * m->width + sx;
  seen[sy * m->width + sx] = true;
  while (head < tail) {
    int at = queue[head++], x = at % m->width, y = at / m->width;
    if (x == tx && y == ty)
      return true;
    const int dx[4] = {0, 0, -1, 1}, dy[4] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; i++) {
      int nx = x + dx[i], ny = y + dy[i], n = ny * m->width + nx;
      const TileDef *t = tile_def(game_tile(g, map, nx, ny));
      if (!t || !t->passable || t->guarded || t->transition || seen[n])
        continue;
      bool npc = false;
      for (int k = 0; k < NPC_COUNT; k++)
        npc |= npcs[k].map == map && npcs[k].x == nx && npcs[k].y == ny;
      if (npc)
        continue;
      seen[n] = true;
      queue[tail++] = n;
    }
  }
  return false;
}
static bool point_reachable(const Game *g, int map, int x, int y) {
  const int entry_x[MAP_COUNT] = {16, 24}, entry_y[MAP_COUNT] = {1, 19};
  const int dx[5] = {0, 0, 0, -1, 1}, dy[5] = {0, -1, 1, 0, 0};
  for (int i = 0; i < 5; i++) {
    int nx = x + dx[i], ny = y + dy[i];
    if (game_passable(g, map, nx, ny) &&
        reachable(g, map, entry_x[map], entry_y[map], nx, ny))
      return true;
  }
  return false;
}

/* The game opens on one page that says how it is played, and waits. */
static int test_title(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  CHECK(g.state == GAME_TITLE);
  /* Walking about does not skip it. */
  game_action(&g, ACT_UP);
  game_action(&g, ACT_LEFT);
  game_action(&g, ACT_INVENTORY);
  CHECK(g.state == GAME_TITLE && g.x == 24 && g.y == 19);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_SCENE_ARRIVAL);
  /* It says what the keys do, and fits the screen. */
  int keys = 0;
  for (int i = 0; i < TITLE_LINES; i++) {
    /* At x=12 the panel holds 38 characters. */
    CHECK(title_page[i] && strlen(title_page[i]) <= 38);
    keys += strstr(title_page[i], "ENTER") != NULL ||
            strstr(title_page[i], "WASD") != NULL || strstr(title_page[i], "ESC") != NULL;
  }
  CHECK(keys >= 3);
  return 0;
}
static int test_world(const char *assets) {
  Game g;
  Map m;
  CHECK(game_init(&g, assets));
  game_action(&g, ACT_CONFIRM); /* past the title page */
  CHECK(!map_load(&m, "no/such/file.map"));
  CHECK(!map_passable(&g.maps[MAP_VILLAGE], -1, 0));
  CHECK(!map_passable(&g.maps[MAP_VILLAGE], 0, 0));
  CHECK(map_passable(&g.maps[MAP_VILLAGE], 16, 21));
  for (int i = 0; i < NPC_COUNT; i++)
    CHECK(map_passable(&g.maps[npcs[i].map], npcs[i].x, npcs[i].y));
  /* The game opens with the arrival scene in the forest. */
  CHECK(g.map == MAP_FOREST && g.state == GAME_DIALOGUE && g.npc == SPEAKER_SCENE);
  CHECK(g.dialogue == D_SCENE_ARRIVAL && dialogues[D_SCENE_ARRIVAL].count == 3);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_DIALOGUE && g.x == 24 && g.y == 19);
  for (int i = 0; i < 3; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && g.obs == 0);
  game_action(&g, ACT_DOWN);
  CHECK(g.x == 24 && g.y == 20);
  int x, y;
  stand(&g, MAP_VILLAGE, 1, 1, 0, 1);
  game_camera(&g, &x, &y);
  CHECK(x == 0 && y == 0);
  stand(&g, MAP_FOREST, 46, 38, 0, 1);
  game_camera(&g, &x, &y);
  CHECK(x == 28 && y == 30);
  /* Transitions both ways land on non-transition tiles. */
  stand(&g, MAP_VILLAGE, 16, 1, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.map == MAP_FOREST && g.x == 24 && g.y == 38);
  game_action(&g, ACT_DOWN);
  CHECK(g.map == MAP_VILLAGE && g.x == 16 && g.y == 1);
  /* NPCs block movement; stepping towards one only turns. */
  stand(&g, MAP_VILLAGE, 5, 5, 0, 1);
  game_action(&g, ACT_UP);
  CHECK(g.x == 5 && g.y == 5 && g.dy == -1);
  return 0;
}

static int test_dialogue_rules(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.npc == NPC_SUMI && g.dialogue == D_SUMI_TASK);
  CHECK(game_knows(&g, OBS_ASKED_BY_SUMI) && g.note_count == 1 && g.notes[0] == N_ASKED);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.page == 1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_WAITING && g.note_count == 1);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION);
  /* Only one of the two marks: no owner story yet. */
  g.obs |= OBS(OBS_HOUSE_MARK);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_WAITING && !game_knows(&g, OBS_BOWL_OWNER));
  game_action(&g, ACT_CANCEL);
  g.obs |= OBS(OBS_BOWL_MARK);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_OWNER && game_knows(&g, OBS_BOWL_OWNER));
  game_action(&g, ACT_CANCEL);
  /* The forbids mask stops the owner story from repeating. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_OWNER_KNOWN);
  game_action(&g, ACT_CANCEL);
  /* Every NPC has a fallback rule. */
  for (int n = 0; n < NPC_COUNT; n++) {
    bool fallback = false;
    for (int i = 0; i < dialogue_rule_count; i++)
      fallback |= (int)dialogue_rules[i].npc == n && dialogue_rules[i].needs == 0;
    CHECK(fallback);
  }
  return 0;
}

static int test_examine(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Examining is idempotent: one observation event, one note. */
  stand(&g, MAP_FOREST, 12, 13, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.npc == -1 && g.examined == 'G');
  CHECK(game_knows(&g, OBS_CLAW_MARKS_EDGE) && g.note_count == 1);
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CANCEL);
  CHECK(g.note_count == 1 && count_events(&g, EV_OBSERVE) == 1);
  CHECK(count_events(&g, EV_EXAMINE) == 2);
  /* The moved stone is a specific point before the generic stone rule. */
  stand(&g, MAP_FOREST, 24, 17, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_DRAGGED && game_knows(&g, OBS_STONE_DRAGGED));
  game_action(&g, ACT_CANCEL);
  /* Passable points are examined underfoot. */
  stand(&g, MAP_FOREST, 24, 12, 1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_HOLLOW && game_knows(&g, OBS_STONE_HOLLOW));
  game_action(&g, ACT_CANCEL);
  /* The offering stone gives the shards once. */
  stand(&g, MAP_FOREST, 38, 21, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_SHARDS && g.player.inventory.quantities[ITEM_SHARDS] == 1);
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_ALTAR_EMPTY && g.player.inventory.quantities[ITEM_SHARDS] == 1);
  game_action(&g, ACT_CANCEL);
  /* The bowl mark text depends on whether the house mark was seen first. */
  game_action(&g, ACT_INVENTORY);
  CHECK(g.state == GAME_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_I_BOWL_MARK && g.examined == 0 && game_knows(&g, OBS_BOWL_MARK));
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_VILLAGE, 4, 7, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_HOUSE_MARK_MATCH && game_knows(&g, OBS_HOUSE_MARK));
  CHECK(g.notes[g.note_count - 1] == N_MARKS_MATCH);
  game_action(&g, ACT_CANCEL);
  return 0;
}

static int test_examine_nothing(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  GameEvent events[EVENT_LIMIT];
  game_take_events(&g, events, EVENT_LIMIT);
  stand(&g, MAP_FOREST, 24, 19, 0, -1);
  Obs before = g.obs;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && g.obs == before && g.note_count == 0);
  CHECK(strstr(g.message, "nichts Besonderes") != NULL);
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  CHECK(count_events(&g, EV_EXAMINE_NOTHING) == 1);
  game_action(&g, ACT_LEFT); /* moves: a step, and a new target */
  game_action(&g, ACT_CONFIRM);
  int taken = game_take_events(&g, events, EVENT_LIMIT);
  int repeats[2] = {-1, -1};
  int nothing = 0, steps = 0;
  for (int i = 0; i < taken; i++) {
    if (events[i].type == EV_EXAMINE_NOTHING && nothing < 2)
      repeats[nothing++] = events[i].b;
    steps += events[i].type == EV_STEP;
  }
  /* The first target was logged once, the second carries its two repeats. */
  CHECK(nothing == 2 && repeats[0] == 0 && repeats[1] == 2);
  CHECK(steps == 1 && g.event_count == 0);
  /* Turning on the spot is also a new target. */
  stand(&g, MAP_VILLAGE, 16, 21, 0, -1);
  game_action(&g, ACT_CONFIRM);
  g.dx = 1;
  g.dy = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(count_events(&g, EV_EXAMINE_NOTHING) == 2);
  /* A full queue drops and counts instead of overwriting. */
  for (int i = 0; i < EVENT_LIMIT + 3; i++) {
    game_action(&g, ACT_CANCEL);
    game_action(&g, ACT_CANCEL);
  }
  CHECK(g.event_count == EVENT_LIMIT && g.events_dropped > 0);
  return 0;
}

static int test_inventory_and_notebook(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  ItemId owned[ITEM_COUNT];
  stand(&g, MAP_VILLAGE, 16, 21, 0, -1);
  CHECK(game_owned_items(&g, owned) == 0);
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_INVENTORY);
  game_action(&g, ACT_INVENTORY);
  CHECK(g.state == GAME_EXPLORATION);
  Inventory inv = {0};
  CHECK(inventory_add(&inv, ITEM_HERB, 99));
  CHECK(!inventory_add(&inv, ITEM_HERB, 1));
  CHECK(inventory_remove(&inv, ITEM_HERB, 99));
  CHECK(!inventory_remove(&inv, ITEM_HERB, 1));
  CHECK(!inventory_add(&inv, ITEM_NONE, 1));
  CHECK(!inventory_add(&inv, ITEM_COUNT, 1));
  CHECK(inventory_add(&g.player.inventory, ITEM_HERB, 1));
  g.player.hp = 23;
  CHECK(player_heal(&g.player) && g.player.hp == 24);
  CHECK(!player_heal(&g.player));
  CHECK(game_owned_items(&g, owned) == 0);
  /* Items without a target nearby do nothing but are logged as attempts. */
  CHECK(inventory_add(&g.player.inventory, ITEM_HERB, 1));
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_INVENTORY && g.player.inventory.quantities[ITEM_HERB] == 1);
  CHECK(count_events(&g, EV_ITEM_USE) == 1 && g.events[g.event_count - 1].b == D_NONE);
  game_action(&g, ACT_CANCEL);
  CHECK(inventory_remove(&g.player.inventory, ITEM_HERB, 1));
  CHECK(inventory_add(&g.player.inventory, ITEM_SHARDS, 1));
  CHECK(game_owned_items(&g, owned) == 1 && owned[0] == ITEM_SHARDS);
  /* Notebook: empty, then scrolling is clamped to the entries. */
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_NOTEBOOK && g.scroll == 0);
  game_action(&g, ACT_UP);
  game_action(&g, ACT_DOWN);
  CHECK(g.scroll == 0);
  game_action(&g, ACT_CANCEL);
  for (int n = N_ASKED; n < NOTE_COUNT; n++)
    g.notes[g.note_count++] = (NoteId)n;
  game_action(&g, ACT_CANCEL);
  CHECK(g.scroll == g.note_count - NOTES_PER_PAGE);
  game_action(&g, ACT_DOWN);
  CHECK(g.scroll == g.note_count - NOTES_PER_PAGE);
  for (int i = 0; i < NOTE_COUNT; i++)
    game_action(&g, ACT_UP);
  CHECK(g.scroll == 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION);
  return 0;
}

static int test_content(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  for (int d = D_NONE + 1; d < DIALOGUE_COUNT; d++) {
    CHECK(dialogues[d].count >= 1 && dialogues[d].count <= DIALOGUE_PAGES);
    for (int page = 0; page < dialogues[d].count; page++)
      CHECK(dialogues[d].pages[page] && text_fits(dialogues[d].pages[page], 3));
  }
  /* Notebook guard (design rule 2): no counters, fractions or checkmarks. */
  for (int n = NOTE_NONE + 1; n < NOTE_COUNT; n++) {
    CHECK(notes[n] && text_fits(notes[n], 2));
    CHECK(strpbrk(notes[n], "0123456789/%#[]*+") == NULL);
  }
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    CHECK(p->dialogue > D_NONE && p->dialogue < DIALOGUE_COUNT);
    if (p->kind == POINT_AT)
      CHECK(point_reachable(&g, p->map, p->x, p->y));
    if (p->kind == POINT_SYMBOL) {
      /* The symbol exists in some state of the world: before or after any
       * override, outcome and phase. */
      bool found = false;
      const Map *m = &g.maps[p->map];
      const Obs states[2] = {0, ~(Obs)0};
      const Obs saved_obs = g.obs;
      const Outcome saved_outcome = g.outcome;
      const Phase saved_phase = g.phase;
      for (int k = 0; k < 2 && !found; k++)
        for (int o = 0; o < OUTCOME_COUNT && !found; o++)
          for (int ph = 0; ph < PHASE_COUNT && !found; ph++) {
            g.obs = states[k];
            g.outcome = (Outcome)o;
            g.phase = (Phase)ph;
            for (int y = 0; y < m->height; y++)
              for (int x = 0; x < m->width; x++)
                if (game_tile(&g, p->map, x, y) == p->symbol &&
                    point_reachable(&g, p->map, x, y))
                  found = true;
          }
      g.obs = saved_obs;
      g.outcome = saved_outcome;
      g.phase = saved_phase;
      CHECK(found);
    }
  }
  g.obs = 0;
  for (int i = 0; i < NPC_COUNT; i++)
    CHECK(point_reachable(&g, npcs[i].map, npcs[i].x, npcs[i].y));
  /* Overrides may block ground (a shelf, the den), but nothing may become
   * unreachable - neither with every observation made, nor with only the ones
   * a single override asks for. */
  for (int i = 0; i < tile_override_count; i++) {
    const TileOverride *o = &tile_overrides[i];
    CHECK(tile_def(o->symbol) != NULL);
    g.obs = o->needs;
    for (int k = 0; k < NPC_COUNT; k++)
      CHECK(point_reachable(&g, npcs[k].map, npcs[k].x, npcs[k].y));
    for (int k = 0; k < examine_point_count; k++)
      if (examine_points[k].kind == POINT_AT)
        CHECK(point_reachable(&g, examine_points[k].map, examine_points[k].x,
                              examine_points[k].y));
  }
  /* The stakes and the mended-bowl pieces are content, so they are checked too. */
  CHECK(tile_def('p') != NULL && tile_def('p')->passable);
  for (int i = 0; i < STAKE_COUNT; i++) {
    g.obs = 0;
    CHECK(game_passable(&g, MAP_FOREST, stakes[i].x, stakes[i].y));
    CHECK(point_reachable(&g, MAP_FOREST, stakes[i].x, stakes[i].y));
    CHECK(game_npc_at(&g, stakes[i].x, stakes[i].y) < 0);
    CHECK(!(stakes[i].x == STONE_HOLLOW_X && stakes[i].y == STONE_HOLLOW_Y));
    for (int k = 0; k < i; k++)
      CHECK(stakes[i].x != stakes[k].x || stakes[i].y != stakes[k].y);
  }
  for (int i = 0; i < place_count; i++)
    for (int t = 0; t < TRANSITION_COUNT; t++) {
      const Place *pl = &places[i];
      const Transition *tr = &transitions[t];
      bool inside = tr->map == pl->map && tr->x >= pl->x && tr->x < pl->x + pl->width &&
                    tr->y >= pl->y && tr->y < pl->y + pl->height;
      CHECK(!inside); /* a room with a map exit would name the wrong place */
    }
  /* The boundary negotiation must not start once something has been decided.
   * (Mending the bowl stays possible: it settles nothing by itself.) */
  for (int i = 0; i < dialogue_rule_count; i++)
    if (dialogue_rules[i].opens == OPEN_FOLLOW)
      CHECK(dialogue_rules[i].outcome == OUT_NONE);
  /* The display order must be a permutation, or the puzzle breaks. */
  for (int i = 0; i < MEND_PIECES; i++) {
    CHECK(mend_display[i] >= 0 && mend_display[i] < MEND_PIECES);
    CHECK(mend_pieces[i].shard && mend_pieces[i].gap);
    for (int k = 0; k < i; k++)
      CHECK(mend_display[i] != mend_display[k]);
  }
  g.obs = ~(Obs)0;
  for (int i = 0; i < NPC_COUNT; i++)
    CHECK(point_reachable(&g, npcs[i].map, npcs[i].x, npcs[i].y));
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_AT)
      CHECK(point_reachable(&g, p->map, p->x, p->y));
  }
  g.obs = 0;
  for (int o = 0; o < OBS_COUNT; o++)
    CHECK(obs_names[o] != NULL);
  /* Log names must cover every action, mood and outcome. */
  for (int i = 0; i < ENC_COUNT; i++)
    CHECK(encounter_action_names[i] != NULL);
  for (int i = 0; i < MOOD_COUNT; i++)
    CHECK(mood_names[i] != NULL);
  for (int i = 0; i < OUTCOME_COUNT; i++)
    CHECK(outcome_names[i] != NULL);
  return 0;
}

static int test_fox_and_tracks(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Tracks are invisible and cannot be examined before the fox is tended. */
  stand(&g, MAP_FOREST, 8, 19, 1, 0);
  CHECK(game_tile(&g, MAP_FOREST, 8, 19) == '.');
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && !game_knows(&g, OBS_TRACKS));
  stand(&g, MAP_FOREST, 6, 20, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FOX && game_knows(&g, OBS_FOX_WOUNDED));
  game_action(&g, ACT_CANCEL);
  /* Mio hands over the herb once. */
  stand(&g, MAP_VILLAGE, 11, 11, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_MIO_HERB && g.player.inventory.quantities[ITEM_HERB] == 1);
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_MIO_HERB_AGAIN && g.player.inventory.quantities[ITEM_HERB] == 1);
  game_action(&g, ACT_CANCEL);
  /* Using the herb elsewhere must not lock the fox out: Mio hands over another. */
  g.player.hp = 10;
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.player.inventory.quantities[ITEM_HERB] == 0 && g.player.hp > 10);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_VILLAGE, 11, 11, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_MIO_HERB_MORE && g.player.inventory.quantities[ITEM_HERB] == 1);
  game_action(&g, ACT_CANCEL);
  /* The herb only tends the fox while facing the den. */
  g.player.hp = g.player.max_hp;
  stand(&g, MAP_FOREST, 6, 21, 0, 1);
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_INVENTORY && !game_knows(&g, OBS_FOX_TENDED));
  CHECK(strcmp(g.message, "Du bist unverletzt.") == 0);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_FOREST, 6, 20, -1, 0);
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_I_TEND_FOX);
  CHECK(game_knows(&g, OBS_FOX_TENDED) && g.player.inventory.quantities[ITEM_HERB] == 0);
  CHECK(g.notes[g.note_count - 1] == N_FOX_TENDED);
  game_action(&g, ACT_CANCEL);
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'f' &&
        game_tile(&g, MAP_FOREST, 8, 19) == 't');
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FOX_TENDED);
  game_action(&g, ACT_CANCEL);
  /* A second herb cannot tend the fox again. */
  CHECK(inventory_add(&g.player.inventory, ITEM_HERB, 1));
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_INVENTORY && g.player.inventory.quantities[ITEM_HERB] == 1);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_FOREST, 8, 19, 1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_TRACKS && game_knows(&g, OBS_TRACKS));
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_VILLAGE, 11, 11, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_MIO_THANKS);
  game_action(&g, ACT_CANCEL);
  return 0;
}

static int options_contain(const Game *g, EncounterAction action) {
  int options[ENCOUNTER_OPTION_LIMIT];
  int count = game_encounter_options(g, options);
  for (int i = 0; i < count; i++)
    if (encounter_options[options[i]].action == action)
      return i + 1;
  return 0;
}
static int test_encounter(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Stepping into the grove starts the encounter; the player stays put. */
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_ENCOUNTER && g.x == 24 && g.y == 12);
  CHECK(g.mood == MOOD_ANGRY && game_knows(&g, OBS_KAMI_SEEN));
  CHECK(count_events(&g, EV_ENCOUNTER) == 1);
  CHECK(strcmp(g.message, dialogues[D_ENC_APPEAR].pages[0]) == 0);
  /* Without anything to offer, only waiting and retreating are possible. */
  CHECK(!options_contain(&g, ENC_OFFER) && options_contain(&g, ENC_WAIT));
  CHECK(options_contain(&g, ENC_RETREAT));
  /* Waiting follows the mood table and does not end the encounter. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_ENCOUNTER && g.mood == MOOD_WARY);
  CHECK(strcmp(g.message, dialogues[D_ENC_WAIT_ANGRY].pages[0]) == 0);
  for (int before = 0; before < MOOD_COUNT; before++) {
    g.mood = (Mood)before;
    g.selection = options_contain(&g, ENC_WAIT) - 1;
    game_action(&g, ACT_CONFIRM);
    CHECK(g.mood == encounter_transitions[ENC_WAIT][before]);
  }
  /* Retreating leaves the grove and pushes the player onto safe ground. */
  g.mood = MOOD_WARY;
  g.selection = options_contain(&g, ENC_RETREAT) - 1;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && g.x == 24 && g.y == 13);
  const TileDef *t = tile_def(game_tile(&g, MAP_FOREST, g.x, g.y));
  CHECK(t->passable && !t->guarded);
  /* The spirit remembers its mood when the player comes back. */
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_ENCOUNTER && g.mood == MOOD_WARY);
  /* Offering the bare shards is possible and makes things worse. */
  CHECK(inventory_add(&g.player.inventory, ITEM_SHARDS, 1));
  int slot = options_contain(&g, ENC_OFFER);
  CHECK(slot > 0);
  g.selection = slot - 1;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.mood == MOOD_ANGRY && game_knows(&g, OBS_KAMI_ANGERED));
  CHECK(g.notes[g.note_count - 1] == N_KAMI_SHARDS);
  CHECK(g.state == GAME_ENCOUNTER);
  CHECK(g.player.inventory.quantities[ITEM_SHARDS] == 1); /* the shards stay */
  /* Escape only highlights retreating. */
  g.selection = 0;
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_ENCOUNTER && options_contain(&g, ENC_RETREAT) - 1 == g.selection);
  /* Every guarded tile bordering open ground releases onto safe ground. */
  for (int y = 1; y < g.maps[MAP_FOREST].height - 1; y++)
    for (int x = 1; x < g.maps[MAP_FOREST].width - 1; x++) {
      if (!tile_def(game_tile(&g, MAP_FOREST, x, y))->guarded ||
          !game_passable(&g, MAP_FOREST, x, y + 1) ||
          tile_def(game_tile(&g, MAP_FOREST, x, y + 1))->guarded)
        continue;
      stand(&g, MAP_FOREST, x, y + 1, 0, -1);
      game_action(&g, ACT_UP);
      CHECK(g.state == GAME_ENCOUNTER);
      g.selection = options_contain(&g, ENC_RETREAT) - 1;
      game_action(&g, ACT_CONFIRM);
      const TileDef *safe = tile_def(game_tile(&g, MAP_FOREST, g.x, g.y));
      CHECK(g.state == GAME_EXPLORATION && g.y > y && safe->passable && !safe->guarded &&
            !safe->transition);
    }
  return 0;
}

/* Choose the encounter option with this action; returns false if it is absent. */
static bool choose(Game *g, EncounterAction action) {
  int slot = options_contain(g, action);
  if (!slot)
    return false;
  g->selection = slot - 1;
  game_action(g, ACT_CONFIRM);
  return true;
}
static int test_fight(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1); /* take the task first */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_TASK);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_ENCOUNTER && !g.fighting && g.outcome == OUT_NONE);
  CHECK(options_contain(&g, ENC_ATTACK) && !options_contain(&g, ENC_HEAL));
  /* Attacking starts the fight and resolves a round at once. */
  CHECK(choose(&g, ENC_ATTACK));
  CHECK(g.fighting && g.state == GAME_ENCOUNTER && g.mood == MOOD_ANGRY);
  CHECK(g.combat.hp < kami.hp && g.player.hp < g.player.max_hp);
  /* During the fight: waiting and offering are gone, the herb appears with one. */
  CHECK(!options_contain(&g, ENC_WAIT) && !options_contain(&g, ENC_OFFER));
  CHECK(!options_contain(&g, ENC_HEAL));
  CHECK(inventory_add(&g.player.inventory, ITEM_HERB, 1));
  CHECK(options_contain(&g, ENC_HEAL) && options_contain(&g, ENC_RETREAT));
  g.player.hp = 8; /* badly hurt: the herb is worth a turn here */
  CHECK(choose(&g, ENC_HEAL));
  CHECK(g.player.inventory.quantities[ITEM_HERB] == 0 && g.player.hp > 8);
  CHECK(strstr(g.message, "Kraut") != NULL);
  /* Stepping back keeps the spirit's wounds. */
  int wounded = g.combat.hp;
  CHECK(choose(&g, ENC_RETREAT));
  CHECK(g.state == GAME_EXPLORATION && !g.fighting && g.outcome == OUT_NONE);
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(choose(&g, ENC_ATTACK));
  CHECK(g.combat.hp < wounded);
  /* Fighting on wins, ends the encounter and settles the grove. */
  for (int round = 0; round < 20 && g.fighting; round++)
    CHECK(choose(&g, ENC_ATTACK));
  CHECK(g.combat.won && g.outcome == OUT_FIGHT && g.state == GAME_DIALOGUE);
  CHECK(g.dialogue == D_ENC_VICTORY && g.npc == SPEAKER_SCENE);
  /* The closing round's numbers survive into the scene. */
  CHECK(strstr(g.message, "Schaden") != NULL &&
        dialogues[D_ENC_VICTORY].pages[0][0] == 0);
  CHECK(g.notes[g.note_count - 1] == N_FOUGHT);
  CHECK(count_events(&g, EV_OUTCOME) == 1);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION);
  CHECK(g.message[0] == 0); /* the status bar does not keep the fight's last line */
  game_action(&g, ACT_UP);
  CHECK(g.y == 11 && g.state == GAME_EXPLORATION); /* the grove is open now */
  /* Sumi reacts to the outcome. */
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_FOUGHT);
  dismiss(&g);
  /* An outcome must not swallow the threads it comes before: the bowl's story
   * is still available afterwards. */
  g.obs |= OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_OWNER && game_knows(&g, OBS_BOWL_OWNER));
  dismiss(&g);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_FOUGHT);
  return 0;
}
/* After an outcome Sumi speaks about it, never about the task it settled -
 * but the bowl's story stays reachable. */
static int test_outcome_keeps_threads(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.outcome = OUT_FIGHT;
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_FOUGHT && !game_knows(&g, OBS_ASKED_BY_SUMI));
  dismiss(&g);
  g.obs |= OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_OWNER && game_knows(&g, OBS_BOWL_OWNER));
  dismiss(&g);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_FOUGHT);
  return 0;
}
static int test_defeat(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  g.player.hp = 3; /* one blow from falling */
  CHECK(choose(&g, ENC_ATTACK));
  CHECK(!g.fighting && g.state == GAME_DIALOGUE && g.outcome == OUT_NONE);
  CHECK(g.dialogue == D_ENC_DEFEAT);
  CHECK(g.map == MAP_VILLAGE && g.x == 16 && g.y == 1);
  CHECK(g.player.hp == g.player.max_hp && g.combat.hp == kami.hp);
  game_action(&g, ACT_CANCEL);
  /* The grove is still guarded: losing decides nothing. */
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_ENCOUNTER && g.y == 12);
  return 0;
}

static int test_boundary(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  CHECK(g.stone_x == STONE_START_X && g.stone_y == STONE_START_Y);
  CHECK(game_tile(&g, MAP_FOREST, STONE_START_X, STONE_START_Y) == 'G');
  /* Without both observations the stone is just a stone in the way. */
  CHECK(!game_can_push(&g));
  stand(&g, MAP_FOREST, 24, 17, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.y == 17 && g.stone_y == STONE_START_Y);
  /* Examining it gives the drag marks; that alone does not unlock pushing. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_DRAGGED && game_knows(&g, OBS_STONE_DRAGGED));
  game_action(&g, ACT_CANCEL);
  CHECK(!game_can_push(&g));
  g.obs |= OBS(OBS_STONE_HOLLOW);
  CHECK(game_can_push(&g));
  /* Pushing moves stone and player one tile each. */
  game_action(&g, ACT_UP);
  CHECK(g.stone_y == STONE_START_Y - 1 && g.y == STONE_START_Y);
  CHECK(game_knows(&g, OBS_STONE_MOVED) && count_events(&g, EV_STONE_PUSH) == 1);
  /* A blocked push moves nothing: the player pushes it sideways into a tree. */
  int sx = g.stone_x, sy = g.stone_y;
  stand(&g, MAP_FOREST, sx + 1, sy, -1, 0);
  while (game_passable(&g, MAP_FOREST, g.stone_x - 1, g.stone_y) && g.stone_x > 2)
    game_action(&g, ACT_LEFT);
  CHECK(!game_passable(&g, MAP_FOREST, g.stone_x - 1, g.stone_y));
  int blocked_x = g.stone_x, px = g.x;
  game_action(&g, ACT_LEFT);
  CHECK(g.stone_x == blocked_x && g.x == px);
  /* The stuck stone can be rolled back to the start of the drag marks. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_STONE_STUCK);
  CHECK(g.stone_x == STONE_START_X && g.stone_y == STONE_START_Y);
  CHECK(!game_knows(&g, OBS_STONE_MOVED));
  game_action(&g, ACT_CANCEL);
  /* Pushing it home settles the boundary. */
  stand(&g, MAP_FOREST, 24, 17, 0, -1);
  for (int i = 0; i < 4; i++)
    game_action(&g, ACT_UP);
  CHECK(g.stone_x == STONE_HOLLOW_X && g.stone_y == STONE_HOLLOW_Y);
  CHECK(g.outcome == OUT_BOUNDARY && g.mood == MOOD_CALM);
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_SCENE_BOUNDARY);
  CHECK(g.notes[g.note_count - 1] == N_BOUNDARY && !game_knows(&g, OBS_STONE_MOVED));
  game_action(&g, ACT_CANCEL);
  /* Settled means settled: no more pushing, and the grove is open. */
  CHECK(!game_can_push(&g));
  stand(&g, MAP_FOREST, 24, 13, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.stone_y == STONE_HOLLOW_Y && g.y == 13);
  stand(&g, MAP_FOREST, 23, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.y == 11 && g.state == GAME_EXPLORATION);
  /* Sumi and Daigo weigh it differently. */
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_BOUNDARY);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_FOREST, 12, 31, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_DAIGO_BOUNDARY);
  return 0;
}

static int test_mend(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Oriha needs the shards and their story before she opens the lacquer. */
  stand(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_ORIHA);
  game_action(&g, ACT_CANCEL);
  g.obs |= OBS(OBS_BOWL_SHARDS);
  CHECK(inventory_add(&g.player.inventory, ITEM_SHARDS, 1));
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_ORIHA_SHARDS);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION); /* no workshop without the story */
  g.obs |= OBS(OBS_BOWL_OWNER);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_ORIHA_MEND);
  for (int i = 0; i < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_MEND && g.mend_placed == 0);
  /* A piece that does not fit costs nothing. */
  int pieces[MEND_PIECES];
  int count = game_mend_pieces(&g, pieces);
  CHECK(count == MEND_PIECES);
  for (int i = 0; i < count; i++)
    if (pieces[i] != 0)
      g.selection = i;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.mend_placed == 0 && g.state == GAME_MEND);
  CHECK(strcmp(g.message, dialogues[D_MEND_WRONG].pages[0]) == 0);
  /* Leaving and coming back keeps the work so far. */
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION);
  game_action(&g, ACT_CONFIRM);
  for (int i = 0; i < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_MEND);
  /* Setting all four pieces in order finishes the bowl. */
  for (int placed = 0; placed < MEND_PIECES; placed++) {
    count = game_mend_pieces(&g, pieces);
    CHECK(count == MEND_PIECES - placed);
    for (int i = 0; i < count; i++)
      if (pieces[i] == placed)
        g.selection = i;
    game_action(&g, ACT_CONFIRM);
    CHECK(g.mend_placed == placed + 1);
  }
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_MEND_DONE);
  CHECK(game_knows(&g, OBS_BOWL_DRYING) && !game_knows(&g, OBS_BOWL_READY));
  CHECK(g.player.inventory.quantities[ITEM_SHARDS] == 0);
  CHECK(g.notes[g.note_count - 1] == N_MENDED);
  game_action(&g, ACT_CANCEL);
  CHECK(game_tile(&g, MAP_VILLAGE, 22, 4) == 'b'); /* it rests on her shelf */
  /* Walking around the village does not dry it; returning from the forest does. */
  stand(&g, MAP_VILLAGE, 16, 2, 0, 1);
  game_action(&g, ACT_DOWN);
  game_action(&g, ACT_UP);
  CHECK(!game_knows(&g, OBS_BOWL_READY));
  stand(&g, MAP_VILLAGE, 16, 1, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.map == MAP_FOREST && !game_knows(&g, OBS_BOWL_READY));
  game_action(&g, ACT_DOWN);
  CHECK(g.map == MAP_VILLAGE && game_knows(&g, OBS_BOWL_READY));
  CHECK(game_tile(&g, MAP_VILLAGE, 22, 4) == 'q');
  /* Oriha hands it over once. */
  stand(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_ORIHA_READY && g.player.inventory.quantities[ITEM_BOWL] == 1);
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_ORIHA_AFTER); /* not "the lacquer needs rest" any more */
  CHECK(g.player.inventory.quantities[ITEM_BOWL] == 1);
  game_action(&g, ACT_CANCEL);
  /* The kami accepts the mended bowl; the shards would have angered it. */
  stand(&g, MAP_FOREST, 24, 12, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_ENCOUNTER && g.mood == MOOD_ANGRY);
  CHECK(game_encounter_offer(&g) == ITEM_BOWL);
  CHECK(choose(&g, ENC_OFFER));
  CHECK(g.mood == MOOD_CALM && game_knows(&g, OBS_KAMI_CALMED));
  CHECK(g.notes[g.note_count - 1] == N_KAMI_CALM);
  CHECK(g.outcome == OUT_NONE); /* the compromise itself is Stage 3E */
  return 0;
}

/* Put the player on a spot with Daigo right behind, as following would. */
static void stand_with_daigo(Game *g, int x, int y) {
  stand(g, MAP_FOREST, x, y, 0, -1);
  g->daigo_x = x;
  g->daigo_y = y + 1;
}
static int test_compromise(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Daigo only comes along once the kami sits and the tracks are known. */
  stand(&g, MAP_FOREST, 12, 31, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_DAIGO && !g.daigo_follows);
  game_action(&g, ACT_CANCEL);
  g.obs |= OBS(OBS_KAMI_CALMED) | OBS(OBS_TRACKS);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_DAIGO && !g.daigo_follows); /* his ledger is still unread */
  game_action(&g, ACT_CANCEL);
  g.obs |= OBS(OBS_LEDGER_DEBT);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_DAIGO_OFFER && !g.daigo_follows);
  CHECK(g.notes[g.note_count - 1] == N_DEAL);
  for (int i = 0; i < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.daigo_follows && game_knows(&g, OBS_DAIGO_DEAL));
  /* He walks in the player's footsteps and never blocks the way. */
  int px = g.x, py = g.y;
  game_action(&g, ACT_DOWN);
  CHECK(g.y == py + 1 && g.daigo_x == px && g.daigo_y == py);
  CHECK(game_npc_at(&g, px, py) == NPC_DAIGO);
  /* Turning back is possible: the follower steps aside. */
  game_action(&g, ACT_UP);
  CHECK(g.x == px && g.y == py);
  /* Stakes go in only where the tracks run. */
  stand_with_daigo(&g, stakes[0].x, stakes[0].y + 2);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.staked == 0);
  game_action(&g, ACT_CANCEL);
  /* And only with Daigo at hand. */
  stand(&g, MAP_FOREST, stakes[0].x, stakes[0].y, 0, -1);
  g.daigo_x = stakes[0].x + 4;
  g.daigo_y = stakes[0].y;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.staked == 0);
  game_action(&g, ACT_CANCEL);
  stand_with_daigo(&g, stakes[0].x, stakes[0].y);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.staked == 1 && g.dialogue == D_STAKE_SET && g.outcome == OUT_NONE);
  CHECK(game_tile(&g, MAP_FOREST, stakes[0].x, stakes[0].y) == 'p');
  game_action(&g, ACT_CANCEL);
  /* A stake is driven in once. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.staked == 1 && g.state != GAME_DIALOGUE);
  stand_with_daigo(&g, stakes[1].x, stakes[1].y);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.staked == 3 && g.outcome == OUT_NONE);
  game_action(&g, ACT_CANCEL);
  stand_with_daigo(&g, stakes[2].x, stakes[2].y);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.outcome == OUT_MEND && g.dialogue == D_SCENE_MEND);
  CHECK(!g.daigo_follows && g.notes[g.note_count - 1] == N_MEND);
  CHECK(count_events(&g, EV_OUTCOME) == 1);
  game_action(&g, ACT_CANCEL);
  /* Both weigh the compromise, each with a cost. */
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_MEND);
  return 0;
}
/* Leaving the forest sends the foreman back to his camp. */
static int test_daigo_stays(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.obs |= OBS(OBS_KAMI_CALMED) | OBS(OBS_TRACKS) | OBS(OBS_LEDGER_DEBT);
  stand(&g, MAP_FOREST, 12, 31, -1, 0);
  game_action(&g, ACT_CONFIRM);
  for (int i = 0; i < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.daigo_follows);
  stand(&g, MAP_FOREST, 24, 38, 0, 1);
  game_action(&g, ACT_DOWN);
  CHECK(g.map == MAP_VILLAGE && !g.daigo_follows);
  CHECK(g.daigo_x == npcs[NPC_DAIGO].x && g.daigo_y == npcs[NPC_DAIGO].y);
  return 0;
}

/* Sleep only settles a decided story, and the morning shows the later change. */
static int test_phases(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  stand(&g, MAP_VILLAGE, 4, 14, 0, -1);
  CHECK(game_tile(&g, MAP_VILLAGE, 4, 14) == 'u');
  game_action(&g, ACT_CONFIRM); /* what it is */
  game_action(&g, ACT_CONFIRM); /* the question */
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.phase == PHASE_BEFORE && g.dialogue == D_X_FUTON_AWAKE);
  game_action(&g, ACT_CANCEL);
  g.outcome = OUT_FIGHT;
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.phase == PHASE_MORNING && g.dialogue == D_SCENE_MORNING);
  CHECK(g.notes[g.note_count - 1] == N_MORNING);
  CHECK(count_events(&g, EV_PHASE) == 1);
  game_action(&g, ACT_CANCEL);
  /* A second night changes nothing more: the futon only describes itself. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FUTON_MORNING && count_events(&g, EV_PHASE) == 1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && count_events(&g, EV_PHASE) == 1);
  return 0;
}
static int test_consequences(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Nothing changes before a decision. */
  for (int i = 0; i < outcome_change_count; i++) {
    const TileOverride *o = &outcome_changes[i];
    CHECK(game_tile(&g, o->map, o->x, o->y) != o->symbol ||
          map_at(&g.maps[o->map], o->x, o->y) == o->symbol);
  }
  /* Each outcome shows a gain and a loss, at once and again in the morning. */
  for (int o = OUT_NONE + 1; o < OUTCOME_COUNT; o++)
    for (int ph = 0; ph < PHASE_COUNT; ph++) {
      bool gain = false, loss = false, visible = true;
      g.outcome = (Outcome)o;
      g.phase = (Phase)ph;
      for (int i = 0; i < outcome_change_count; i++) {
        const TileOverride *c = &outcome_changes[i];
        if (c->outcome != (Outcome)o || (c->phase != PHASE_ANY && c->phase != (Phase)ph))
          continue;
        bool differs = map_at(&g.maps[c->map], c->x, c->y) != c->symbol;
        gain |= c->tag == TAG_GAIN && differs;
        loss |= c->tag == TAG_LOSS && differs;
        visible &= differs && game_tile(&g, c->map, c->x, c->y) == c->symbol;
      }
      CHECK(gain && loss && visible);
    }
  /* The world stays walkable in every state. */
  for (int o = 0; o < OUTCOME_COUNT; o++)
    for (int ph = 0; ph < PHASE_COUNT; ph++) {
      g.outcome = (Outcome)o;
      g.phase = (Phase)ph;
      g.obs = ~(Obs)0;
      for (int i = 0; i < NPC_COUNT; i++)
        CHECK(point_reachable(&g, npcs[i].map, npcs[i].x, npcs[i].y));
      CHECK(point_reachable(&g, MAP_FOREST, 37, 19)); /* the shrine and its patch */
    }
  return 0;
}
/* After the conflict Sumi asks, and the night is available from her and the
 * futon; both use the same transition, and it happens only once. */
static int test_night_offer(const char *assets) {
  const Outcome outcomes[3] = {OUT_FIGHT, OUT_BOUNDARY, OUT_MEND};
  for (int i = 0; i < 3; i++) {
    Game g;
    CHECK(game_init(&g, assets));
    g.outcome = outcomes[i];
    stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
    /* Her reaction ends with the question. */
    game_action(&g, ACT_CONFIRM);
    for (int page = 0; page < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; page++)
      game_action(&g, ACT_CONFIRM);
    CHECK(g.state == GAME_PROMPT && g.dialogue == D_PROMPT_SLEEP);
    /* Staying keeps the player where they are, the night stays available. */
    g.selection = 1;
    game_action(&g, ACT_CONFIRM);
    CHECK(g.state == GAME_EXPLORATION && g.phase == PHASE_BEFORE);
    CHECK(g.x == 5 && g.y == 5);
    /* Escape declines as well. */
    game_action(&g, ACT_CONFIRM);
    for (int page = 0; page < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; page++)
      game_action(&g, ACT_CONFIRM);
    CHECK(g.state == GAME_PROMPT);
    game_action(&g, ACT_CANCEL);
    CHECK(g.state == GAME_EXPLORATION && g.phase == PHASE_BEFORE);
    /* Later, from her again: this time the night. */
    game_action(&g, ACT_CONFIRM);
    for (int page = 0; page < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; page++)
      game_action(&g, ACT_CONFIRM);
    CHECK(g.state == GAME_PROMPT);
    g.selection = 0;
    game_action(&g, ACT_CONFIRM);
    CHECK(g.phase == PHASE_MORNING && g.dialogue == D_SCENE_MORNING);
    CHECK(g.map == MAP_VILLAGE && game_tile(&g, MAP_VILLAGE, g.x, g.y) == 'u');
    CHECK(g.notes[g.note_count - 1] == N_MORNING);
    CHECK(count_events(&g, EV_PHASE) == 1);
    game_action(&g, ACT_CANCEL);
    /* No second morning, and no question about it any more. */
    stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
    game_action(&g, ACT_CONFIRM);
    for (int page = 0; page < DIALOGUE_PAGES && g.state == GAME_DIALOGUE; page++)
      game_action(&g, ACT_CONFIRM);
    CHECK(g.state == GAME_EXPLORATION && count_events(&g, EV_PHASE) == 1);
  }
  return 0;
}
/* The village says what its houses are, from the first visit on. */
static int test_signs(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* The inn sign, before anything at all has happened. */
  stand(&g, MAP_VILLAGE, 5, 17, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_INN_SIGN && g.outcome == OUT_NONE);
  game_action(&g, ACT_CANCEL);
  /* The workshop sign and her materials. */
  stand(&g, MAP_VILLAGE, 24, 7, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_WORKSHOP_SIGN);
  game_action(&g, ACT_CANCEL);
  stand(&g, MAP_VILLAGE, 26, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_WORKBENCH);
  game_action(&g, ACT_CANCEL);
  /* Stepping inside names the place for a moment. */
  stand(&g, MAP_VILLAGE, 6, 16, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.place && strcmp(g.place, "Gasthaus von Kiriyama") == 0 && g.place_ticks > 0);
  stand(&g, MAP_VILLAGE, 25, 6, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.place && strcmp(g.place, "Orihas Lackwerkstatt") == 0);
  /* Sumi's house carries her name only once the player has met her. */
  stand(&g, MAP_VILLAGE, 6, 6, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.place == NULL || strcmp(g.place, "Sumis Haus") != 0);
  g.obs |= OBS(OBS_ASKED_BY_SUMI);
  stand(&g, MAP_VILLAGE, 6, 6, 0, -1);
  game_action(&g, ACT_UP);
  CHECK(g.place && strcmp(g.place, "Sumis Haus") == 0);
  /* The name fades on its own. */
  for (int i = 0; i < 64 && g.place_ticks > 0; i++)
    game_action(&g, ACT_NONE);
  CHECK(g.place_ticks == 0);
  return 0;
}
/* Examining the futon explains it; only the answer tries to sleep. */
static int test_futon_explains(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  stand(&g, MAP_VILLAGE, 5, 14, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FUTON && g.state == GAME_DIALOGUE);
  CHECK(g.phase == PHASE_BEFORE && count_events(&g, EV_PHASE) == 0);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION); /* escape just closes the description */
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_PROMPT); /* the question, not a night */
  /* Unsolved: trying to sleep says why, and nothing changes. */
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FUTON_AWAKE && g.phase == PHASE_BEFORE);
  game_action(&g, ACT_CANCEL);
  /* Declining leaves the player standing there. */
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_PROMPT);
  g.selection = 1;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION && g.phase == PHASE_BEFORE);
  /* Settled: the same answer now leads into the morning. */
  g.outcome = OUT_BOUNDARY;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_PROMPT);
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.phase == PHASE_MORNING && g.dialogue == D_SCENE_MORNING);
  return 0;
}
/* The futon works from its own tile and from the one in front of it, and never
 * claims the forest is restless once the conflict is settled. */
static int test_futon(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  /* Before any outcome: the attempt is refused, with a reason. */
  stand(&g, MAP_VILLAGE, 5, 14, -1, 0);
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_PROMPT);
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FUTON_AWAKE && g.phase == PHASE_BEFORE);
  game_action(&g, ACT_CANCEL);
  /* Settled: from the tile in front of it. */
  g.outcome = OUT_MEND;
  game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CONFIRM);
  g.selection = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.phase == PHASE_MORNING && g.dialogue == D_SCENE_MORNING);
  CHECK(game_tile(&g, MAP_VILLAGE, g.x, g.y) == 'u');
  game_action(&g, ACT_CANCEL);
  CHECK(g.dialogue != D_X_FUTON_AWAKE);
  /* Standing on it in the morning says so, and changes nothing. */
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_FUTON_MORNING && count_events(&g, EV_PHASE) == 1);
  game_action(&g, ACT_CANCEL);
  /* And once settled, sleeping from its own tile works too. */
  Game h;
  CHECK(game_init(&h, assets));
  h.outcome = OUT_FIGHT;
  stand(&h, MAP_VILLAGE, 4, 14, 0, -1);
  game_action(&h, ACT_CONFIRM);
  CHECK(h.dialogue == D_X_FUTON);
  game_action(&h, ACT_CONFIRM);
  CHECK(h.state == GAME_PROMPT);
  h.selection = 0;
  game_action(&h, ACT_CONFIRM);
  CHECK(h.phase == PHASE_MORNING && h.dialogue == D_SCENE_MORNING);
  return 0;
}
/* Reading the shrine in the morning shows the line about the visitor first,
 * and only then the closing scene. */
static int test_visitor_before_teaser(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.outcome = OUT_BOUNDARY;
  g.phase = PHASE_MORNING;
  g.obs |= OBS(OBS_MORNING) | OBS(OBS_GREY_TRACE);
  stand(&g, MAP_FOREST, 37, 19, 1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_INSCRIPTION_LATE);
  CHECK(g.notes[g.note_count - 1] == N_VISITOR);
  for (int page = 0; page < DIALOGUE_PAGES && g.dialogue == D_X_INSCRIPTION_LATE; page++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_SCENE_TEASER);
  CHECK(game_knows(&g, OBS_TEASED));
  return 0;
}
/* The den with kits describes itself, like the empty one does. */
static int test_den_with_kits(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.outcome = OUT_MEND;
  g.phase = PHASE_MORNING;
  g.obs |= OBS(OBS_MORNING) | OBS(OBS_FOX_TENDED);
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'g');
  stand(&g, MAP_FOREST, 6, 20, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.dialogue == D_X_FOX_KITS);
  return 0;
}
/* What an outcome changes wins over what was true before it. */
static int test_change_precedence(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.obs |= OBS(OBS_FOX_TENDED);
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'f');
  g.outcome = OUT_FIGHT;
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'f'); /* it stays the day of the fight */
  g.phase = PHASE_MORNING;
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'e'); /* by morning it has left */
  g.outcome = OUT_MEND;
  CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'g'); /* and came back with kits */
  return 0;
}
/* What the foreman says right after each outcome, and the next morning. */
static int test_daigo_reactions(const char *assets) {
  const Outcome outcomes[3] = {OUT_FIGHT, OUT_BOUNDARY, OUT_MEND};
  const DialogueId today[3] = {D_DAIGO_FOUGHT, D_DAIGO_BOUNDARY, D_DAIGO_MEND};
  const DialogueId morning[3] = {D_DAIGO_MORNING_FIGHT, D_DAIGO_MORNING_BOUNDARY,
                                 D_DAIGO_MORNING_MEND};
  for (int i = 0; i < 3; i++)
    for (int ledger = 0; ledger < 2; ledger++) {
      Game g;
      CHECK(game_init(&g, assets));
      g.outcome = outcomes[i];
      if (ledger)
        g.obs |= OBS(OBS_LEDGER_DEBT); /* his book must not change the answer */
      stand(&g, MAP_FOREST, 12, 31, -1, 0);
      game_action(&g, ACT_CONFIRM);
      CHECK(g.dialogue == (int)today[i]);
      dismiss(&g);
      g.phase = PHASE_MORNING;
      g.obs |= OBS(OBS_MORNING);
      game_action(&g, ACT_CONFIRM);
      CHECK(g.dialogue == (int)morning[i]);
      dismiss(&g);
    }
  return 0;
}
/* Once something is decided he no longer opens a different way out. */
static int test_no_late_deal(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.obs |= OBS(OBS_KAMI_CALMED) | OBS(OBS_TRACKS) | OBS(OBS_LEDGER_DEBT);
  stand(&g, MAP_FOREST, 12, 31, -1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_DAIGO_OFFER); /* undecided: the offer stands */
  dismiss(&g);
  CHECK(g.daigo_follows);
  Game h;
  CHECK(game_init(&h, assets));
  h.obs = g.obs;
  h.outcome = OUT_FIGHT;
  stand(&h, MAP_FOREST, 12, 31, -1, 0);
  game_action(&h, ACT_CONFIRM);
  CHECK(h.dialogue == D_DAIGO_FOUGHT && !h.daigo_follows);
  dismiss(&h);
  /* Not even once the deal had been struck before the kami fell. */
  h.obs |= OBS(OBS_DAIGO_DEAL);
  game_action(&h, ACT_CONFIRM);
  CHECK(h.dialogue == D_DAIGO_FOUGHT && !h.daigo_follows);
  return 0;
}
/* After the fight the fox stays for the day and is gone the next morning, with
 * fresh felling around the den to show why. Mio remembers who helped it. */
static int test_fox_after_fight(const char *assets) {
  for (int tended = 0; tended < 2; tended++) {
    Game g;
    CHECK(game_init(&g, assets));
    g.outcome = OUT_FIGHT;
    g.obs |= OBS(OBS_FOX_WOUNDED);
    if (tended) {
      g.obs |= OBS(OBS_FOX_TENDED) | OBS(OBS_TRACKS);
      g.notes[g.note_count++] = N_FOX_TENDED;
    }
    /* The day of the fight: the den is as the player left it. */
    CHECK(game_tile(&g, MAP_FOREST, 5, 20) == (tended ? 'f' : 'F'));
    stand(&g, MAP_FOREST, 6, 20, -1, 0);
    game_action(&g, ACT_CONFIRM);
    CHECK(g.dialogue == (tended ? D_X_FOX_TENDED : D_X_FOX));
    dismiss(&g);
    /* The next morning: empty, with wood chips and fresh stumps nearby. */
    g.phase = PHASE_MORNING;
    g.obs |= OBS(OBS_MORNING);
    CHECK(game_tile(&g, MAP_FOREST, 5, 20) == 'e'); /* tending does not override it */
    int stumps = 0;
    for (int y = 17; y <= 23; y++)
      for (int x = 2; x <= 11; x++)
        stumps += game_tile(&g, MAP_FOREST, x, y) == 'x';
    CHECK(stumps >= 2);
    game_action(&g, ACT_CONFIRM);
    CHECK(g.dialogue == D_X_DEN_EMPTY);
    dismiss(&g);
    /* What the player did for the fox is still part of the story. */
    if (tended) {
      CHECK(game_knows(&g, OBS_TRACKS) && game_tile(&g, MAP_FOREST, 8, 19) == 't');
      bool kept = false;
      for (int i = 0; i < g.note_count; i++)
        kept |= g.notes[i] == N_FOX_TENDED;
      CHECK(kept);
    }
    /* And Mio speaks to it. */
    stand(&g, MAP_VILLAGE, 11, 11, 0, -1);
    game_action(&g, ACT_CONFIRM);
    CHECK(g.dialogue == (tended ? D_MIO_MORNING_FIGHT_HELPED : D_MIO_MORNING_FIGHT));
    dismiss(&g);
  }
  return 0;
}
/* The morning must not swallow the bowl's story either. */
static int test_morning_keeps_threads(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  g.outcome = OUT_BOUNDARY;
  g.phase = PHASE_MORNING;
  g.obs |= OBS(OBS_MORNING) | OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK);
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_OWNER && game_knows(&g, OBS_BOWL_OWNER));
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_SUMI_MORNING_BOUNDARY);
  return 0;
}
/* The grey patch appears after any outcome and says nothing about itself. */
static int test_grey_trace(const char *assets) {
  Game g;
  CHECK(game_init(&g, assets));
  CHECK(game_tile(&g, MAP_FOREST, 37, 19) != 'v');
  g.outcome = OUT_BOUNDARY;
  CHECK(game_tile(&g, MAP_FOREST, 37, 19) == 'v');
  stand(&g, MAP_FOREST, 37, 20, 0, -1);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_TRACE && game_knows(&g, OBS_GREY_TRACE));
  CHECK(g.notes[g.note_count - 1] == N_TRACE);
  game_action(&g, ACT_CANCEL);
  /* Reading tracks adds what is missing, not an explanation. */
  g.obs &= ~OBS(OBS_GREY_TRACE);
  g.obs |= OBS(OBS_FOX_TENDED);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_TRACE_TRACKS);
  game_action(&g, ACT_CANCEL);
  /* Only then does the shrine show the line about a visitor. */
  stand(&g, MAP_FOREST, 37, 19, 1, 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == D_X_INSCRIPTION_LATE);
  CHECK(g.notes[g.note_count - 1] == N_VISITOR);
  return 0;
}

static int test_combat(void) {
  for (int a = 0; a < 12; a++)
    for (int d = 0; d < 12; d++)
      for (int r = -1; r <= 1; r++) {
        int hit = combat_damage(a, d, r);
        CHECK(hit >= 1 && hit < 14);
      }
  Combat c;
  Player p = {.hp = 24, .max_hp = 24, .attack = 8, .defense = 2};
  char message[128];
  combat_begin(&c);
  for (int turn = 0; turn < 20 && !c.won && !c.lost; turn++)
    combat_turn(&c, &p, 0, false, message, sizeof message);
  CHECK(c.won && !c.lost && p.hp > 0);
  combat_begin(&c);
  p.hp = 1;
  combat_turn(&c, &p, 0, false, message, sizeof message);
  CHECK(c.lost && p.hp == 0);
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;
  if (test_title(argv[1]) || test_world(argv[1]) || test_dialogue_rules(argv[1]) ||
      test_examine(argv[1]) || test_examine_nothing(argv[1]) ||
      test_inventory_and_notebook(argv[1]) || test_content(argv[1]) ||
      test_fox_and_tracks(argv[1]) || test_encounter(argv[1]) || test_fight(argv[1]) ||
      test_outcome_keeps_threads(argv[1]) || test_defeat(argv[1]) ||
      test_boundary(argv[1]) || test_mend(argv[1]) || test_compromise(argv[1]) ||
      test_daigo_stays(argv[1]) || test_phases(argv[1]) || test_consequences(argv[1]) ||
      test_night_offer(argv[1]) || test_signs(argv[1]) || test_futon_explains(argv[1]) ||
      test_futon(argv[1]) || test_change_precedence(argv[1]) ||
      test_visitor_before_teaser(argv[1]) || test_den_with_kits(argv[1]) ||
      test_fox_after_fight(argv[1]) || test_daigo_reactions(argv[1]) ||
      test_no_late_deal(argv[1]) || test_morning_keeps_threads(argv[1]) ||
      test_grey_trace(argv[1]) || test_combat())
    return 1;
  puts("World, examining, encounter, outcomes, consequences, morning, trace pass.");
  return 0;
}
