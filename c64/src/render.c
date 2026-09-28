#include <c64.h>
#include "render.h"
#include "screen.h"
#include "world.h"

#define VIEW_H 17     /* map rows, below the status line */
#define PANEL_TOP 18  /* the separator; the panel is what follows it */
#define ENCOUNTER_TOP 13 /* the encounter needs room for text, round and options */
#define MEND_TOP 15

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
static uint8_t prev_map = 0xff, prev_x, prev_y, prev_outcome, prev_stone_x,
    prev_stone_y, prev_staked;
static Obs prev_obs;
static int8_t prev_cx, prev_cy;
static bool state_changed(const Game *g) {
  return map_stale || prev_map != g->map || prev_obs != g->obs ||
         prev_outcome != g->outcome ||
         prev_stone_x != (uint8_t)g->stone_x ||
         prev_stone_y != (uint8_t)g->stone_y ||
         prev_staked != g->staked;
}
static bool camera_unchanged(const Game *g) {
  int8_t cx, cy;
  game_camera(g, SCREEN_COLS, VIEW_H, &cx, &cy);
  if (cx != prev_cx || cy != prev_cy)
    return false;
  prev_cx = cx;
  prev_cy = cy;
  return true;
}
/* Restores a cell to what game_tile() resolves, when the player is not there
 * and no NPC stands in it. */
static void cell_restore(const Game *g, uint8_t x, uint8_t y) {
  const TileDef *t = 0;
  if (g->map == MAP_FOREST && x == (uint8_t)g->stone_x && y == (uint8_t)g->stone_y)
    t = tile_def('G');
  else if (g->map == MAP_FOREST)
    for (uint8_t i = 0; i < STAKE_COUNT; i++)
      if ((g->staked & (1u << i)) && stakes[i].x == x && stakes[i].y == y)
        t = tile_def('p');
  if (!t)
    for (uint8_t i = tile_override_count; i > 0; i--) {
      const TileOverride *o = &tile_overrides[i - 1];
      if (o->map == g->map && o->x == x && o->y == y && game_shows(g, o)) {
        t = tile_def(o->symbol);
        break;
      }
    }
  if (!t)
    t = tile_def(map_at(g->map, (int8_t)x, (int8_t)y));
  screen_put(x, y, t ? t->screen : 32, t ? t->color : COLOR_BLACK);
}
static bool npc_at_cell(const Game *g, uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < NPC_COUNT; i++) {
    int8_t nx, ny;
    game_npc_pos(g, i, &nx, &ny);
    if (npcs[i].map == g->map && nx == (int8_t)x && ny == (int8_t)y)
      return true;
  }
  return false;
}
static void map_view(const Game *g) {
  if (!state_changed(g) && prev_x == (uint8_t)g->x && prev_y == (uint8_t)g->y) {
    prev_x = (uint8_t)g->x;
    prev_y = (uint8_t)g->y;
    map_stale = false;
    return;
  }
  if (!state_changed(g) && camera_unchanged(g) &&
      !npc_at_cell(g, prev_x, prev_y) && !npc_at_cell(g, (uint8_t)g->x, (uint8_t)g->y)) {
    /* Only the player moved: restore the old cell, draw the new one. */
    uint8_t ovx = (uint8_t)(prev_x - prev_cx), ovy = (uint8_t)(prev_y - prev_cy);
    if (ovx < SCREEN_COLS && ovy < VIEW_H)
      cell_restore(g, ovx, (uint8_t)(ovy + 1));
    uint8_t nvx = (uint8_t)((uint8_t)g->x - prev_cx), nvy = (uint8_t)((uint8_t)g->y - prev_cy);
    if (nvx < SCREEN_COLS && nvy < VIEW_H)
      screen_put(nvx, (uint8_t)(nvy + 1), 0 /* '@' */, COLOR_WHITE);
    prev_x = (uint8_t)g->x;
    prev_y = (uint8_t)g->y;
    map_stale = false;
    return;
  }
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
  /* Rueckwaerts, damit am Ende die erste passende Ueberschreibung oben liegt --
   * genau die, die game_tile() auch nimmt (Regal: fertig schlaegt trocknend). */
  for (uint8_t i = tile_override_count; i > 0; i--) {
    const TileOverride *o = &tile_overrides[i - 1];
    if (o->map == g->map && game_shows(g, o))
      in_view(cx, cy, o->x, o->y, o->symbol);
  }
  for (uint8_t i = 0; i < NPC_COUNT; i++) {
    int8_t nx, ny;
    game_npc_pos(g, i, &nx, &ny);
    if (npcs[i].map == g->map)
      overlay(cx, cy, (uint8_t)nx, (uint8_t)ny, (uint8_t)npcs[i].glyph, COLOR_CYAN);
  }
  overlay(cx, cy, (uint8_t)g->x, (uint8_t)g->y, 0 /* '@' */, COLOR_WHITE);
  prev_map = g->map;
  prev_x = (uint8_t)g->x;
  prev_y = (uint8_t)g->y;
  prev_obs = g->obs;
  prev_outcome = g->outcome;
  prev_stone_x = (uint8_t)g->stone_x;
  prev_stone_y = (uint8_t)g->stone_y;
  prev_staked = g->staked;
  prev_cx = cx;
  prev_cy = cy;
  map_stale = false;
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
  screen_text(0, y, "TASCHE", COLOR_YELLOW);
  screen_invert(0, y++, 6); /* damit offen auch offen aussieht */
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
  if (g->fighting) { /* the bar is the player's own state, so it says so */
    screen_text(28, y, "DU", COLOR_WHITE);
    /* Zehn Felder ohne Division: das i-te ist voll, sobald hp*10 die Schwelle
     * PLAYER_HP*(i+1) erreicht -- dasselbe wie hp*10/PLAYER_HP > i (#25). */
    for (uint8_t i = 0; i < 10; i++) /* a bar, not a number: no counters */
      screen_put((uint8_t)(31 + i), y,
                 (uint8_t)(g->hp * 10 >= PLAYER_HP * (i + 1) ? 160 : 45),
                 COLOR_LIGHTGREEN);
  }
  y++;
  const Dialogue *d = &dialogues[g->dialogue];
  const char *text = g->page < d->count ? d->pages[g->page] : "";
  y = screen_lines(0, y, *text ? text : g->message, COLOR_WHITE);
  /* What the round did belongs on screen next to what was said. */
  if (*text && g->message[0])
    y = screen_lines(0, y, g->message, COLOR_LIGHTGREEN);
  for (uint8_t i = 0; i < count && y < SCREEN_ROWS - 1; i++) {
    screen_row(y, COLOR_WHITE);
    screen_text(0, y, i == g->selection ? ">" : " ", COLOR_YELLOW);
    screen_text(2, y++, encounter_options[options[i]].label, COLOR_WHITE);
  }
  blank(y);
  hint("W/S waehlen   RETURN tun");
}

/* The repair: the gap says what is missing, the pieces lie beside the bowl.
 * Nothing counts anything down -- the gaps do that by being there. */
static void mend_panel(const Game *g) {
  uint8_t pieces[MEND_PIECES];
  uint8_t count = game_mend_pieces(g, pieces), y = MEND_TOP + 1;
  map_stale = true; /* the panel covers the lower rows of the map */
  separator(MEND_TOP);
  screen_row(y, COLOR_YELLOW);
  screen_text(0, y++, "Die Schale", COLOR_YELLOW);
  if (g->mend_placed < MEND_PIECES) {
    screen_row(y, COLOR_WHITE);
    screen_text(0, y++, mend_pieces[g->mend_placed].gap, COLOR_WHITE);
  }
  if (g->message[0])
    y = screen_lines(0, y, g->message, COLOR_LIGHTGREEN);
  for (uint8_t i = 0; i < count && y < SCREEN_ROWS - 1; i++) {
    screen_row(y, COLOR_WHITE);
    screen_text(0, y, i == g->selection ? ">" : " ", COLOR_YELLOW);
    screen_text(2, y++, mend_pieces[pieces[i]].shard, COLOR_WHITE);
  }
  blank(y);
  hint("W/S waehlen   RETURN setzen");
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

/* Die Schlusstafel: was entschieden wurde, und was der Ausschnitt offen laesst. */
static void closing(const Game *g) {
  map_stale = true;
  screen_clear();
  screen_text(3, 2, closing_page[0], COLOR_YELLOW);
  screen_text(3, 4, closing_line[g->outcome], COLOR_LIGHTGREEN);
  for (uint8_t i = 2; i < CLOSING_LINES - 1; i++)
    screen_text(3, (uint8_t)(4 + i), closing_page[i], COLOR_WHITE);
  hint(closing_page[CLOSING_LINES - 1]); /* wie ueberall unten am Rand */
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
  if (g->state == GAME_END) {
    closing(g);
    return;
  }
  status_line(g);
  map_view(g);
  if (g->state == GAME_ENCOUNTER) {
    encounter_panel(g);
    return;
  }
  if (g->state == GAME_MEND) {
    mend_panel(g);
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
