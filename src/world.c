#include "world.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const TileDef tiles[] = {
    {.symbol = '.', .passable = true, .transition = false, .name = "Wiese", .art = 0},
    {.symbol = ',', .passable = true, .transition = false, .name = "Weg", .art = 1},
    {.symbol = '~', .passable = false, .transition = false, .name = "Bach", .art = 2},
    {.symbol = 'T', .passable = false, .transition = false, .name = "Kiefer", .art = 3},
    {.symbol = '^', .passable = false, .transition = false, .name = "Fels", .art = 4},
    {.symbol = '#',
     .passable = false,
     .transition = false,
     .name = "Dorfmauer",
     .art = 5},
    {.symbol = 'H', .passable = false, .transition = false, .name = "Hauswand", .art = 6},
    {.symbol = '_', .passable = true, .transition = false, .name = "Boden", .art = 7},
    {.symbol = '+', .passable = true, .transition = false, .name = "Tuer", .art = 8},
    {.symbol = '>',
     .passable = true,
     .transition = true,
     .name = "Weg ins Dorf",
     .art = 9},
    {.symbol = '<', .passable = true, .transition = true, .name = "Nordtor", .art = 10},
    {.symbol = '*',
     .passable = true,
     .transition = false,
     .name = "Steinlaterne",
     .art = 11},
    {.symbol = 'S', .passable = false, .transition = false, .name = "Sakura", .art = 19},
    {.symbol = 'B', .passable = false, .transition = false, .name = "Bambus", .art = 20},
    {.symbol = 'r',
     .passable = false,
     .transition = false,
     .name = "Reisfeld",
     .art = 21},
    {.symbol = '=', .passable = true, .transition = false, .name = "Bruecke", .art = 22},
    {.symbol = 'R',
     .passable = false,
     .transition = false,
     .name = "Ziegeldach",
     .art = 23},
    {.symbol = 'l',
     .passable = false,
     .transition = false,
     .name = "Papierlaterne",
     .art = 24},
    {.symbol = 's', .passable = true, .transition = false, .name = "Kies", .art = 25},
    {.symbol = '[',
     .passable = false,
     .transition = false,
     .name = "Dachkante",
     .art = 26},
    {.symbol = ']',
     .passable = false,
     .transition = false,
     .name = "Dachkante",
     .art = 27},
    {.symbol = 'G',
     .passable = false,
     .transition = false,
     .name = "Grenzstein",
     .art = 28},
    {.symbol = 'x',
     .passable = false,
     .transition = false,
     .name = "Baumstumpf",
     .art = 29},
    {.symbol = 'O', .passable = false, .transition = false, .name = "Schrein", .art = 30},
    {.symbol = 'o',
     .passable = false,
     .transition = false,
     .name = "Opferstein",
     .art = 31},
    {.symbol = 'Y',
     .passable = false,
     .transition = false,
     .name = "Alter Baum",
     .art = 32},
    {.symbol = 'm', .passable = true, .transition = false, .name = "Mulde", .art = 33},
    {.symbol = 'd',
     .passable = true,
     .transition = false,
     .name = "Aufgewuehlte Erde",
     .art = 34},
    {.symbol = 'A', .passable = false, .transition = false, .name = "Zelt", .art = 35},
    {.symbol = 'k',
     .passable = false,
     .transition = false,
     .name = "Werktisch",
     .art = 36},
    {.symbol = 'W',
     .passable = false,
     .transition = false,
     .name = "Holzstapel",
     .art = 37},
    {.symbol = 'M',
     .passable = false,
     .transition = false,
     .name = "Hauswand",
     .art = 38},
    {.symbol = 'h',
     .passable = true,
     .transition = false,
     .name = "Moosboden",
     .guarded = true,
     .art = 39},
    {.symbol = 'F',
     .passable = false,
     .transition = false,
     .name = "Fuchsbau",
     .art = 41},
    {.symbol = 'f',
     .passable = false,
     .transition = false,
     .name = "Fuchsbau",
     .art = 42},
    {.symbol = 't', .passable = true, .transition = false, .name = "Faehrte", .art = 43},
    {.symbol = 'b', .passable = false, .transition = false, .name = "Regal", .art = 44},
    {.symbol = 'q', .passable = false, .transition = false, .name = "Regal", .art = 45},
    {.symbol = 'p',
     .passable = true,
     .transition = false,
     .name = "Grenzpfahl",
     .art = 46},
    {.symbol = 'u', .passable = true, .transition = false, .name = "Futon", .art = 47},
    {.symbol = 'e',
     .passable = false,
     .transition = false,
     .name = "Leerer Bau",
     .art = 48},
    {.symbol = 'g',
     .passable = false,
     .transition = false,
     .name = "Fuchsbau",
     .art = 49},
    {.symbol = 'j',
     .passable = false,
     .transition = false,
     .name = "Setzling",
     .art = 50},
    {.symbol = 'v',
     .passable = false,
     .transition = false,
     .name = "Graue Spur",
     .art = 51},
    {.symbol = 'n', .passable = false, .transition = false, .name = "Schild", .art = 52},
    {.symbol = 'w', .passable = false, .transition = false, .name = "Schild", .art = 53},
    {.symbol = 'c',
     .passable = false,
     .transition = false,
     .name = "Werkbank",
     .art = 54},
    {.symbol = 'z',
     .passable = false,
     .transition = false,
     .name = "Totholzstapel",
     .art = 55},
    {.symbol = 'P',
     .passable = true,
     .transition = false,
     .name = "Markierte Stelle",
     .art = 56}};
const int tile_count = (int)(sizeof tiles / sizeof tiles[0]);
const TileDef *tile_def(char s) {
  for (int i = 0; i < tile_count; i++)
    if (tiles[i].symbol == s)
      return &tiles[i];
  return NULL;
}
char map_at(const Map *m, int x, int y) {
  if (x < 0 || y < 0 || x >= m->width || y >= m->height)
    return '^';
  return m->cells[y * m->width + x];
}
bool map_passable(const Map *m, int x, int y) {
  const TileDef *t = tile_def(map_at(m, x, y));
  return t && t->passable;
}
/* Reads one decimal number and moves `*s` past it; false if there is none. */
static bool read_int(const char **s, int *out) {
  char *end;
  errno = 0;
  long v = strtol(*s, &end, 10);
  if (end == *s || errno == ERANGE || v < INT_MIN || v > INT_MAX)
    return false;
  *s = end;
  *out = (int)v;
  return true;
}
bool map_load(Map *m, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;
  Map next = {0};
  int w, h;
  char line[128];
  const char *cursor = line;
  if (!fgets(line, sizeof line, f) || !read_int(&cursor, &w) || !read_int(&cursor, &h) ||
      w < 1 || h < 1 || w > MAP_LIMIT || h > MAP_LIMIT) {
    fclose(f);
    return false;
  }
  next.width = (uint8_t)w;
  next.height = (uint8_t)h;
  for (int y = 0; y < h; y++) {
    if (!fgets(line, sizeof line, f) || strcspn(line, "\r\n") != (size_t)w) {
      fclose(f);
      return false;
    }
    for (int x = 0; x < w; x++) {
      if (!tile_def(line[x])) {
        fclose(f);
        return false;
      }
      next.cells[y * w + x] = line[x];
    }
  }
  fclose(f);
  *m = next;
  return true;
}
