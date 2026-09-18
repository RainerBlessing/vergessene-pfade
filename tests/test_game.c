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

static int test_world(const char *assets) {
  Game g;
  Map m;
  CHECK(game_init(&g, assets));
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
  game_action(&g, ACT_LEFT); /* moves: a new target */
  game_action(&g, ACT_CONFIRM);
  CHECK(game_take_events(&g, events, EVENT_LIMIT) == 2);
  CHECK(events[0].b == 0 && events[1].type == EV_EXAMINE_NOTHING && events[1].b == 2);
  CHECK(g.event_count == 0);
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
      CHECK(text_fits(dialogues[d].pages[page], 3));
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
      /* The symbol exists before or after overrides (all observations known). */
      bool found = false;
      const Map *m = &g.maps[p->map];
      const Obs states[2] = {0, ~(Obs)0};
      const Obs saved = g.obs;
      for (int k = 0; k < 2; k++) {
        g.obs = states[k];
        for (int y = 0; y < m->height; y++)
          for (int x = 0; x < m->width; x++)
            if (game_tile(&g, p->map, x, y) == p->symbol &&
                point_reachable(&g, p->map, x, y))
              found = true;
      }
      g.obs = saved;
      CHECK(found);
    }
  }
  g.obs = 0;
  for (int i = 0; i < NPC_COUNT; i++)
    CHECK(point_reachable(&g, npcs[i].map, npcs[i].x, npcs[i].y));
  /* Overrides replace passable ground with passable ground or the fox den. */
  for (int i = 0; i < tile_override_count; i++) {
    const TileOverride *o = &tile_overrides[i];
    const TileDef *base = tile_def(map_at(&g.maps[o->map], o->x, o->y));
    const TileDef *now = tile_def(o->symbol);
    CHECK(base && now && (base->passable == now->passable || o->symbol == 'f'));
  }
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
  CHECK(g.notes[g.note_count - 1] == N_FOUGHT);
  CHECK(count_events(&g, EV_OUTCOME) == 1);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION);
  game_action(&g, ACT_UP);
  CHECK(g.y == 11 && g.state == GAME_EXPLORATION); /* the grove is open now */
  /* Sumi reacts to the outcome. */
  stand(&g, MAP_VILLAGE, 5, 5, 0, -1);
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
  if (test_world(argv[1]) || test_dialogue_rules(argv[1]) || test_examine(argv[1]) ||
      test_examine_nothing(argv[1]) || test_inventory_and_notebook(argv[1]) ||
      test_content(argv[1]) || test_fox_and_tracks(argv[1]) || test_encounter(argv[1]) ||
      test_fight(argv[1]) || test_defeat(argv[1]) || test_combat())
    return 1;
  puts("World, dialogue, examining, notebook, content, fox, encounter, fight pass.");
  return 0;
}
