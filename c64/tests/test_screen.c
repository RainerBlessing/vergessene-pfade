/* What the player actually reads: the panel layout, checked on the host with
 * screen RAM pointed at plain arrays. */
#include <c64.h>
#include "../src/render.h"
#include "../src/screen.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint8_t test_screen[1000], test_color[1000];
struct host_vic VIC;

/* Back from screen codes to readable text. */
static void row_text(uint8_t y, char *out) {
  for (uint8_t x = 0; x < SCREEN_COLS; x++) {
    uint8_t c = test_screen[y * SCREEN_COLS + x];
    out[x] = c == 0            ? '@'
             : c >= 1 && c <= 26 ? (char)(c + 96)
             : c >= 65 && c <= 90 ? (char)c
             : c >= 32 && c < 64  ? (char)c
                                  : '#';
  }
  out[SCREEN_COLS] = 0;
  for (int8_t i = SCREEN_COLS - 1; i >= 0 && out[i] == ' '; i--)
    out[i] = 0;
}
static void expect_row(uint8_t y, const char *text) {
  char seen[SCREEN_COLS + 1];
  row_text(y, seen);
  if (strcmp(seen, text)) {
    printf("Zeile %u:\n  erwartet: \"%s\"\n  gesehen:  \"%s\"\n", y, text, seen);
    fflush(stdout);
    assert(0);
  }
}
static void start(Game *g) {
  game_init(g);
  game_action(g, ACT_CONFIRM);
  while (g->state == GAME_DIALOGUE)
    game_action(g, ACT_CONFIRM);
}

/* A dialogue page has to appear line by line, whole. */
static void dialogue_pages_are_complete(void) {
  Game g;
  start(&g);
  g.map = MAP_VILLAGE;
  g.x = 5;
  g.y = 5;
  g.dx = 0;
  g.dy = -1;
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_TASK);
  render(&g);
  expect_row(19, "Sumi / Dorfaelteste");
  expect_row(20, "Du bist zurueck. Gut. Wir brauchen");
  expect_row(21, "jede Hand. Im Wald greift ein");
  expect_row(22, "Geist unsere Holzfaeller an.");
  expect_row(24, "RETURN weiter");
  game_action(&g, ACT_CONFIRM);
  render(&g);
  expect_row(20, "Ohne Holz frieren wir im Winter.");
  expect_row(21, "Geh in den Wald und vertreibe");
  expect_row(22, "den Geist. Bitte.");
}

/* N opens the notebook from the map, and from an open text in two presses. */
static void notebook_opens(void) {
  Game g;
  start(&g);
  game_action(&g, ACT_CANCEL);
  assert(g.state == GAME_NOTEBOOK);
  render(&g);
  expect_row(0, "NOTIZBUCH");
  game_action(&g, ACT_CANCEL);
  assert(g.state == GAME_EXPLORATION);
  /* The map has to come back, not stay blank behind the closed notebook. */
  render(&g);
  char seen[SCREEN_COLS + 1];
  row_text(9, seen);
  assert(strlen(seen) > 0);
}

static void notebook_shows_what_was_written(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST; /* the wounded fox, seen from below its den */
  g.x = 5;
  g.y = 21;
  g.dx = 0;
  g.dy = -1;
  game_action(&g, ACT_CONFIRM);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  game_action(&g, ACT_CANCEL);
  render(&g);
  expect_row(3, "  Ein verletzter Fuchs am Bau. An");
  expect_row(4, "  der Pfote dunkler Lehm.");
}

/* The repair view: the gap names what is missing, the pieces lie beside it. */
static void mend_view_shows_gap_and_pieces(void) {
  Game g;
  start(&g);
  g.state = GAME_MEND;
  g.map = MAP_VILLAGE;
  g.x = 24;
  g.y = 5;
  g.selection = 0;
  render(&g);
  expect_row(16, "Die Schale");
  expect_row(17, "Der Boden fehlt.");
  expect_row(18, "> Randstueck");
  expect_row(19, "  Bodenstueck");
  expect_row(24, "W/S waehlen   RETURN setzen");
  /* A piece that does not fit says so and changes nothing. */
  game_action(&g, ACT_CONFIRM);
  assert(g.mend_placed == 0);
  render(&g);
  expect_row(18, "Das passt nicht an diese");
  expect_row(19, "Bruchkante.");
}

/* In der Begegnung steht neben dem Text auch, was die Runde gebracht hat. */
static void encounter_shows_the_round(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST;
  g.x = 25;
  g.y = 12;
  g.dx = 0;
  g.dy = -1;
  game_action(&g, ACT_UP);
  assert(g.state == GAME_ENCOUNTER);
  render(&g);
  expect_row(14, "Waldkami    ZORNIG");
  expect_row(15, "Zwischen den Staemmen richtet sich");
  /* Angreifen: der Text bleibt, die Zahlen der Runde kommen darunter. */
  uint8_t options[ENCOUNTER_OPTION_LIMIT];
  uint8_t count = game_encounter_options(&g, options);
  for (uint8_t i = 0; i < count; i++)
    if (encounter_options[options[i]].action == ENC_ATTACK)
      g.selection = i;
  game_action(&g, ACT_CONFIRM);
  render(&g);
  expect_row(15, "Du machst einen Schritt nach vorn.");
  char seen[SCREEN_COLS + 1];
  row_text(17, seen);
  assert(strstr(seen, "Dein Hieb verursacht") == seen);
  row_text(14, seen);
  assert(strstr(seen, "DU") != 0); /* der Balken ist der eigene */
}

/* Der Balken rechnet ohne Division (#25) -- hier steht, dass er trotzdem
 * dieselben Stufen zeigt. Er beginnt in Spalte 31; gezaehlt werden die Felder,
 * die auf dem Bild liegen (bis Spalte 39). Volles Feld ist 160. */
#define BAR_FROM 31
static uint8_t bar_filled(const Game *g) {
  render(g);
  uint8_t n = 0;
  for (uint8_t x = BAR_FROM; x < SCREEN_COLS; x++)
    if (test_screen[14 * SCREEN_COLS + x] == 160)
      n++;
  return n;
}
static void the_health_bar_keeps_its_steps(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST;
  g.x = 25;
  g.y = 12;
  g.dx = 0;
  g.dy = -1;
  game_action(&g, ACT_UP);
  g.fighting = true;
  assert(g.hp == PLAYER_HP);
  assert(bar_filled(&g) == SCREEN_COLS - BAR_FROM); /* voll ist voll */
  g.hp = PLAYER_HP / 2;
  assert(bar_filled(&g) == 5);
  g.hp = 3; /* ein Zehntel von 24 aufwaerts: das erste Feld */
  assert(bar_filled(&g) == 1);
  g.hp = 0;
  assert(bar_filled(&g) == 0);
}

/* Was gezeichnet wird, muss dem entsprechen, was game_tile() sagt -- sonst
 * zeigt die Karte etwas anderes als das Spiel meint (Regal: trocknend/fertig). */
static void drawn_map_matches_the_rules(const Game *g) {
  int8_t cx, cy;
  game_camera(g, SCREEN_COLS, 17 /* Kartenzeilen, siehe render.c */, &cx, &cy);
  for (uint8_t vy = 0; vy < 17; vy++)
    for (uint8_t vx = 0; vx < SCREEN_COLS; vx++) {
      int8_t x = (int8_t)(cx + vx), y = (int8_t)(cy + vy);
      if ((uint8_t)x >= map_width(g->map) || (uint8_t)y >= map_height(g->map))
        continue;
      if (game_npc_at(g, x, y) >= 0 || (x == g->x && y == g->y))
        continue;
      const TileDef *t = tile_def(game_tile(g, g->map, x, y));
      uint8_t drawn = test_screen[(vy + 1) * SCREEN_COLS + vx];
      if (!t || drawn == t->screen)
        continue;
      printf("Feld %d,%d: gezeichnet %u, erwartet %u (%c)\n", x, y, drawn, t->screen,
             game_tile(g, g->map, x, y));
      fflush(stdout);
      assert(0);
    }
}

static void the_shelf_shows_the_dried_bowl(void) {
  Game g;
  start(&g);
  g.map = MAP_VILLAGE;
  g.x = 24;
  g.y = 6;
  g.obs |= OBS(OBS_BOWL_DRYING);
  render(&g);
  assert(game_tile(&g, MAP_VILLAGE, 22, 4) == 'b');
  drawn_map_matches_the_rules(&g);
  g.obs |= OBS(OBS_BOWL_READY); /* beides gesetzt: fertig gewinnt */
  render(&g);
  assert(game_tile(&g, MAP_VILLAGE, 22, 4) == 'q');
  drawn_map_matches_the_rules(&g);
}

/* Die offene Tasche hebt sich ab und laesst sich mit einem Schritt verlassen. */
static void the_open_bag_is_marked(void) {
  Game g;
  start(&g);
  g.bag[ITEM_HERB] = 1;
  game_action(&g, ACT_INVENTORY);
  render(&g);
  expect_row(20, "> Heilkraut");
  for (uint8_t x = 0; x < 6; x++)
    assert(test_screen[19 * SCREEN_COLS + x] >= 128); /* TASCHE steht invers */
}

/* Die Hausmarke neben Sumis Tuer muss sich von der Wand unterscheiden, sonst
 * findet sie niemand (#15). */
static void the_house_mark_stands_out(void) {
  const TileDef *wall = tile_def('H');
  const TileDef *mark = tile_def('M');
  assert(wall && mark);
  assert(mark->screen != wall->screen || mark->color != wall->color);
  Game g;
  start(&g);
  g.map = MAP_VILLAGE;
  g.x = 4;
  g.y = 7;
  render(&g);
  int8_t cx, cy;
  game_camera(&g, SCREEN_COLS, 17, &cx, &cy);
  uint16_t at = (uint16_t)((6 - cy + 1) * SCREEN_COLS + (4 - cx));
  assert(test_screen[at] == mark->screen);
  assert(test_color[at] == mark->color);
}

/* Die Schlusstafel nennt den Ausgang und sagt, dass hier Schluss ist (#22). */
static void the_closing_panel_names_the_ending(void) {
  Game g;
  start(&g);
  g.state = GAME_END;
  g.outcome = OUT_MEND;
  render(&g);
  expect_row(2, "   HIER ENDET DER AUSSCHNITT");
  expect_row(4, "   Drei Pfaehle und eine Schale im Moos.");
  expect_row(6, "   In der vollen Fassung folgt eine");
  expect_row(24, "RETURN: weiterlaufen");
}

/* Player-only move (camera static): old cell restored, new cell drawn. */
static void player_move_camera_static(void) {
  Game g;
  start(&g);
  g.map = MAP_VILLAGE;
  g.x = 12;
  g.y = 10;
  g.dx = 1;
  g.dy = 0;
  render(&g);
  int8_t cx, cy;
  game_camera(&g, SCREEN_COLS, 17, &cx, &cy);
  /* Save the full screen buffer after the initial render. */
  uint8_t before[SCREEN_COLS * SCREEN_ROWS];
  memcpy(before, test_screen, sizeof(before));
  /* Move right: (12,10) -> (13,10). Village is 32 wide < 40 viewport,
   * so the camera stays at (0,2) for all x positions. */
  g.x = 13;
  render(&g);
  int8_t cx2, cy2;
  game_camera(&g, SCREEN_COLS, 17, &cx2, &cy2);
  assert(cx == cx2 && cy == cy2); /* camera truly unchanged */
  /* Old player cell (12,10) must be restored to its base tile. */
  uint16_t old_at = (uint16_t)((10 - cy + 1) * SCREEN_COLS + (12 - cx));
  uint8_t old_char = (uint8_t)map_at(MAP_VILLAGE, 12, 10);
  const TileDef *old_def = tile_def(old_char);
  assert(test_screen[old_at] == old_def->screen);
  assert(test_color[old_at] == old_def->color);
  /* New player cell (13,10) must show '@'. */
  uint16_t new_at = (uint16_t)((10 - cy + 1) * SCREEN_COLS + (13 - cx));
  assert(test_screen[new_at] == 0);
  assert(test_color[new_at] == COLOR_WHITE);
  /* All other cells must be unchanged. */
  for (uint16_t i = 0; i < SCREEN_COLS * SCREEN_ROWS; i++) {
    if (i == old_at || i == new_at)
      continue;
    assert(test_screen[i] == before[i]);
  }
  drawn_map_matches_the_rules(&g);
}

/* Der Schnellpfad zeichnet nur zwei Felder -- dann muss aber auch alles andere
 * noch stimmen, was game_tile() sagt. Die verlassene Zelle liegt hier ueber
 * einer Zeile, die anders aussieht: eine Verwechslung von Bild- und
 * Kartenzeile faellt damit auf. */
static void player_move_restores_the_right_tile(void) {
  Game g;
  start(&g);
  g.map = MAP_VILLAGE;
  g.x = 12;
  g.y = 1; /* darunter, in Zeile 2, steht bei x=12 eine Sakura */
  g.dx = 1;
  g.dy = 0;
  render(&g);
  int8_t cx, cy;
  game_camera(&g, SCREEN_COLS, 17, &cx, &cy);
  g.x = 13;
  render(&g);
  int8_t cx2, cy2;
  game_camera(&g, SCREEN_COLS, 17, &cx2, &cy2);
  assert(cx == cx2 && cy == cy2); /* wirklich der Schnellpfad */
  drawn_map_matches_the_rules(&g);
}

/* Player move that changes the camera: full redraw. */
static void player_move_camera_moves(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST;
  g.x = 24;
  g.y = 20;
  g.dx = 1;
  g.dy = 0;
  render(&g);
  int8_t cx1, cy1;
  game_camera(&g, SCREEN_COLS, 17, &cx1, &cy1);
  /* Move right: (24,20) -> (25,20). Camera shifts from 4 to 5. */
  g.x = 25;
  render(&g);
  int8_t cx2, cy2;
  game_camera(&g, SCREEN_COLS, 17, &cx2, &cy2);
  assert(cx1 != cx2 || cy1 != cy2); /* camera actually moved */
  /* Player must be visible at the correct position. */
  uint16_t at = (uint16_t)((20 - cy2 + 1) * SCREEN_COLS + (25 - cx2));
  assert(test_screen[at] == 0);
  assert(test_color[at] == COLOR_WHITE);
}

/* NPC on the new player cell: full redraw (npc_at_cell guard). */
static void npc_on_cell_forces_full_redraw(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST;
  g.x = 5;
  g.y = 20;
  g.dx = 1;
  g.dy = 0;
  render(&g);
  /* Sumi stands at (16,20) in the forest. Move player toward her. */
  g.x = 15;
  render(&g);
  int8_t cx, cy;
  game_camera(&g, SCREEN_COLS, 17, &cx, &cy);
  /* Move right into Sumi's cell: (15,20) -> (16,20).
   * npc_at_cell(g, 16, 20) is true -> full redraw path. */
  g.x = 16;
  render(&g);
  /* Player must be on top of the NPC. */
  game_camera(&g, SCREEN_COLS, 17, &cx, &cy);
  uint16_t at = (uint16_t)((20 - cy + 1) * SCREEN_COLS + (16 - cx));
  assert(test_screen[at] == 0); /* '@' on top of NPC */
  assert(test_color[at] == COLOR_WHITE);
}

static void the_map_is_drawn(void) {
  Game g;
  start(&g);
  render(&g);
  char seen[SCREEN_COLS + 1];
  row_text(0, seen);
  assert(strcmp(seen, "Der Wald") == 0);
  /* The player stands in the middle of the view. */
  assert(test_screen[(1 + 8) * SCREEN_COLS + 20] == 0); /* '@' */
}

int main(void) {
  the_map_is_drawn();
  dialogue_pages_are_complete();
  notebook_opens();
  notebook_shows_what_was_written();
  mend_view_shows_gap_and_pieces();
  encounter_shows_the_round();
  the_health_bar_keeps_its_steps();
  the_shelf_shows_the_dried_bowl();
  the_open_bag_is_marked();
  player_move_camera_static();
  player_move_restores_the_right_tile();
  player_move_camera_moves();
  npc_on_cell_forces_full_redraw();
  the_house_mark_stands_out();
  the_closing_panel_names_the_ending();
  printf("Bildschirm-Tests bestanden\n");
  return 0;
}
