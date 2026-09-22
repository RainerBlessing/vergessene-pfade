#include <c64.h>
#include "render.h"
#include "screen.h"

#define VIEW_H 17     /* map rows, below the status line */
#define PANEL_TOP 18  /* the separator; the panel is what follows it */
#define ENCOUNTER_TOP 15

static void separator(uint8_t y) {
  for (uint8_t x = 0; x < SCREEN_COLS; x++)
    screen_put(x, y, 64, COLOR_GRAY1); /* horizontal line */
}
static void blank(uint8_t from) {
  for (uint8_t y = from; y < SCREEN_ROWS; y++)
    screen_row(y, COLOR_WHITE);
}
static void hint(const char *text) {
  screen_row(SCREEN_ROWS - 1, COLOR_GRAY2);
  screen_text(0, SCREEN_ROWS - 1, text, COLOR_GRAY2);
}

static void status_line(const Game *g) {
  screen_row(0, COLOR_YELLOW);
  if (g->place_ticks && g->place)
    screen_text(0, 0, g->place, COLOR_YELLOW);
  else
    screen_text(0, 0, g->map == MAP_VILLAGE ? "Kiriyama" : "Der Wald", COLOR_GRAY2);
  if (g->note_ticks)
    screen_text(24, 0, "Notizbuch: neu", COLOR_LIGHTGREEN);
}

/* Draws a single cell of the map, if the camera can see it. */
static void overlay(int8_t cx, int8_t cy, uint8_t x, uint8_t y, uint8_t code,
                    uint8_t color) {
  if (x < (uint8_t)cx || y < (uint8_t)cy)
    return;
  uint8_t vx = (uint8_t)(x - cx), vy = (uint8_t)(y - cy);
  if (vx < SCREEN_COLS && vy < VIEW_H)
    screen_put(vx, (uint8_t)(vy + 1), code, color);
}
static void in_view(int8_t cx, int8_t cy, uint8_t x, uint8_t y, char symbol) {
  const TileDef *t = tile_def(symbol);
  if (t)
    overlay(cx, cy, x, y, t->screen, t->color);
}

/* The map is the expensive part of a frame: 680 cells on a 1 MHz machine.
 * So the base map is drawn straight from its rows, and everything that changes
 * it -- overrides, the moved stone, the people, the player -- is put on top
 * afterwards, instead of being asked about once per cell. */
static void put_symbol(uint8_t x, uint8_t y, char symbol) {
  const TileDef *t = tile_def(symbol);
  screen_put(x, y, t ? t->screen : 32, t ? t->color : COLOR_WHITE);
}
/* Redrawing the map costs far more than the panel below it, so it happens only
 * when something up there can have changed. */
static bool map_stale = true; /* something was drawn over the map area */
static bool map_is_current(const Game *g) {
  static uint8_t map = 0xff, x, y, outcome, stone_x, stone_y;
  static Obs obs;
  if (!map_stale && map == g->map && x == (uint8_t)g->x && y == (uint8_t)g->y &&
      obs == g->obs && outcome == g->outcome &&
      stone_x == (uint8_t)g->stone_x && stone_y == (uint8_t)g->stone_y)
    return true;
  map = g->map;
  x = (uint8_t)g->x;
  y = (uint8_t)g->y;
  obs = g->obs;
  outcome = g->outcome;
  stone_x = (uint8_t)g->stone_x;
  stone_y = (uint8_t)g->stone_y;
  map_stale = false;
  return false;
}
static void map_view(const Game *g) {
  if (map_is_current(g))
    return;
  int8_t cx, cy;
  game_camera(g, SCREEN_COLS, VIEW_H, &cx, &cy);
  uint8_t width = map_width(g->map), height = map_height(g->map);
  for (uint8_t vy = 0; vy < VIEW_H; vy++) {
    uint8_t y = (uint8_t)(cy + vy);
    if (y >= height) {
      screen_row((uint8_t)(vy + 1), COLOR_WHITE);
      continue;
    }
    const char *row = map_row(g->map, y);
    for (uint8_t vx = 0; vx < SCREEN_COLS; vx++) {
      uint8_t x = (uint8_t)(cx + vx);
      if (x < width) /* a map narrower than the screen keeps a black margin */
        put_symbol(vx, (uint8_t)(vy + 1), row[x]);
      else
        screen_put(vx, (uint8_t)(vy + 1), 32, COLOR_BLACK);
    }
  }
  /* On top of the map, in the order game_tile() resolves them. */
  if (g->map == MAP_FOREST)
    in_view(cx, cy, (uint8_t)g->stone_x, (uint8_t)g->stone_y, 'G');
  for (uint8_t i = 0; i < tile_override_count; i++) {
    const TileOverride *o = &tile_overrides[i];
    if (o->map == g->map && game_shows(g, o))
      in_view(cx, cy, o->x, o->y, o->symbol);
  }
  for (uint8_t i = 0; i < NPC_COUNT; i++)
    if (npcs[i].map == g->map)
      overlay(cx, cy, npcs[i].x, npcs[i].y, (uint8_t)npcs[i].glyph, COLOR_CYAN);
  overlay(cx, cy, (uint8_t)g->x, (uint8_t)g->y, 0 /* '@' */, COLOR_WHITE);
}

/* Who is speaking: a person, a scene, or the thing being looked at. */
static const char *speaker(const Game *g) {
  if (g->npc == SPEAKER_SCENE)
    return g->scene;
  if (g->npc >= 0)
    return npcs[g->npc].name;
  const TileDef *t = g->examined ? tile_def(g->examined) : 0;
  return t ? t->name : "Du siehst hin";
}

static void dialogue_panel(const Game *g, uint8_t top) {
  const Dialogue *d = &dialogues[g->dialogue];
  const char *text = g->page < d->count ? d->pages[g->page] : "";
  screen_row(top + 1, COLOR_YELLOW);
  screen_text(0, top + 1, speaker(g), COLOR_YELLOW);
  /* A page left empty shows the message instead: the last round of a fight. */
  uint8_t next = screen_lines(0, top + 2, *text ? text : g->message, COLOR_WHITE);
  blank(next);
  hint(g->page + 1 < d->count ? "RETURN weiter" : "RETURN schliessen");
}

static void exploration_panel(const Game *g, uint8_t top) {
  uint8_t next = screen_lines(0, top + 1, g->message, COLOR_LIGHTGREEN);
  blank(next);
  hint("RETURN ansehen   I Tasche   N Notizbuch");
}

static void inventory_panel(const Game *g, uint8_t top) {
  uint8_t owned[ITEM_COUNT];
  uint8_t count = game_owned_items(g, owned), y = top + 1;
  screen_row(y, COLOR_YELLOW);
  screen_text(0, y++, "TASCHE", COLOR_YELLOW);
  if (!count) {
    screen_row(y, COLOR_WHITE);
    screen_text(2, y++, "Nichts dabei.", COLOR_WHITE);
  }
  for (uint8_t i = 0; i < count; i++) {
    screen_row(y, COLOR_WHITE);
    screen_text(0, y, i == g->selection ? ">" : " ", COLOR_YELLOW);
    screen_text(2, y++, items[owned[i]].name, COLOR_WHITE);
  }
  blank(y);
  hint("RETURN benutzen   I schliessen");
}

static void encounter_panel(const Game *g) {
  map_stale = true; /* the panel covers the lower rows of the map */
  uint8_t options[ENCOUNTER_OPTION_LIMIT];
  uint8_t count = game_encounter_options(g, options), y = ENCOUNTER_TOP + 1;
  separator(ENCOUNTER_TOP);
  screen_row(y, COLOR_LIGHTRED);
  screen_text(0, y, "Waldkami", COLOR_LIGHTRED);
  screen_text(12, y, mood_names[g->mood], COLOR_LIGHTRED);
  if (g->fighting) {
    screen_text(28, y, "LP", COLOR_WHITE);
    for (uint8_t i = 0; i < 10; i++) /* a bar, not a number: no counters */
      screen_put((uint8_t)(31 + i), y, (uint8_t)(g->hp * 10 / PLAYER_HP > i ? 160 : 45),
                 COLOR_LIGHTGREEN);
  }
  y++;
  const Dialogue *d = &dialogues[g->dialogue];
  const char *text = g->page < d->count ? d->pages[g->page] : "";
  y = screen_lines(0, y, *text ? text : g->message, COLOR_WHITE);
  for (uint8_t i = 0; i < count && y < SCREEN_ROWS - 1; i++) {
    screen_row(y, COLOR_WHITE);
    screen_text(0, y, i == g->selection ? ">" : " ", COLOR_YELLOW);
    screen_text(2, y++, encounter_options[options[i]].label, COLOR_WHITE);
  }
  blank(y);
  hint("W/S waehlen   RETURN tun");
}

static void notebook(const Game *g) {
  map_stale = true;
  screen_clear();
  screen_text(0, 0, "NOTIZBUCH", COLOR_YELLOW);
  separator(1);
  uint8_t y = 3;
  if (!g->note_count)
    screen_text(2, y, "Noch nichts aufgeschrieben.", COLOR_WHITE);
  for (uint8_t i = 0; i < NOTES_PER_PAGE; i++) {
    uint8_t at = (uint8_t)(g->scroll + i);
    if (at >= g->note_count)
      break;
    y = screen_lines(2, y, notes[g->notes[at]], COLOR_WHITE);
    y++;
  }
  hint("W/S blaettern   N schliessen");
}

static void title(void) {
  map_stale = true;
  screen_clear();
  for (uint8_t i = 0; i < TITLE_LINES; i++)
    screen_text(3, (uint8_t)(3 + i), title_page[i], i < 2 ? COLOR_YELLOW : COLOR_WHITE);
}

void render(const Game *g) {
  if (g->state == GAME_TITLE) {
    title();
    return;
  }
  if (g->state == GAME_NOTEBOOK) {
    notebook(g);
    return;
  }
  status_line(g);
  map_view(g);
  if (g->state == GAME_ENCOUNTER) {
    encounter_panel(g);
    return;
  }
  separator(PANEL_TOP);
  if (g->state == GAME_DIALOGUE)
    dialogue_panel(g, PANEL_TOP);
  else if (g->state == GAME_INVENTORY)
    inventory_panel(g, PANEL_TOP);
  else
    exploration_panel(g, PANEL_TOP);
}
