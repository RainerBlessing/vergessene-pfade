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
  printf("Bildschirm-Tests bestanden\n");
  return 0;
}
