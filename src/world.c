#include "world.h"
#include <stdio.h>
#include <string.h>
const TileDef tiles[] = {
    {.symbol = '.', .passable = true, .transition = false, .name = "Wiese"},
    {.symbol = ',', .passable = true, .transition = false, .name = "Weg"},
    {.symbol = '~', .passable = false, .transition = false, .name = "Bach"},
    {.symbol = 'T', .passable = false, .transition = false, .name = "Kiefer"},
    {.symbol = '^', .passable = false, .transition = false, .name = "Fels"},
    {.symbol = '#', .passable = false, .transition = false, .name = "Dorfmauer"},
    {.symbol = 'H', .passable = false, .transition = false, .name = "Hauswand"},
    {.symbol = '_', .passable = true, .transition = false, .name = "Boden"},
    {.symbol = '+', .passable = true, .transition = false, .name = "Tuer"},
    {.symbol = '>', .passable = true, .transition = true, .name = "Weg ins Dorf"},
    {.symbol = '<', .passable = true, .transition = true, .name = "Nordtor"},
    {.symbol = '*', .passable = true, .transition = false, .name = "Steinlaterne"},
    {.symbol = 'S', .passable = false, .transition = false, .name = "Sakura"},
    {.symbol = 'B', .passable = false, .transition = false, .name = "Bambus"},
    {.symbol = 'r', .passable = false, .transition = false, .name = "Reisfeld"},
    {.symbol = '=', .passable = true, .transition = false, .name = "Bruecke"},
    {.symbol = 'R', .passable = false, .transition = false, .name = "Ziegeldach"},
    {.symbol = 'l', .passable = false, .transition = false, .name = "Papierlaterne"},
    {.symbol = 's', .passable = true, .transition = false, .name = "Kies"},
    {.symbol = '[', .passable = false, .transition = false, .name = "Dachkante"},
    {.symbol = ']', .passable = false, .transition = false, .name = "Dachkante"},
    {.symbol = 'G', .passable = false, .transition = false, .name = "Grenzstein"},
    {.symbol = 'x', .passable = false, .transition = false, .name = "Baumstumpf"},
    {.symbol = 'O', .passable = false, .transition = false, .name = "Schrein"},
    {.symbol = 'o', .passable = false, .transition = false, .name = "Opferstein"},
    {.symbol = 'Y', .passable = false, .transition = false, .name = "Alter Baum"},
    {.symbol = 'm', .passable = true, .transition = false, .name = "Mulde"},
    {.symbol = 'd', .passable = true, .transition = false, .name = "Aufgewuehlte Erde"},
    {.symbol = 'A', .passable = false, .transition = false, .name = "Zelt"},
    {.symbol = 'k', .passable = false, .transition = false, .name = "Werktisch"},
    {.symbol = 'W', .passable = false, .transition = false, .name = "Holzstapel"},
    {.symbol = 'M', .passable = false, .transition = false, .name = "Hauswand"},
    {.symbol = 'h',
     .passable = true,
     .transition = false,
     .name = "Moosboden",
     .guarded = true},
    {.symbol = 'F', .passable = false, .transition = false, .name = "Fuchsbau"},
    {.symbol = 'f', .passable = false, .transition = false, .name = "Fuchsbau"},
    {.symbol = 't', .passable = true, .transition = false, .name = "Faehrte"},
    {.symbol = 'b', .passable = false, .transition = false, .name = "Regal"},
    {.symbol = 'q', .passable = false, .transition = false, .name = "Regal"},
    {.symbol = 'p', .passable = true, .transition = false, .name = "Grenzpfahl"},
    {.symbol = 'u', .passable = true, .transition = false, .name = "Futon"},
    {.symbol = 'e', .passable = false, .transition = false, .name = "Leerer Bau"},
    {.symbol = 'g', .passable = false, .transition = false, .name = "Fuchsbau"},
    {.symbol = 'j', .passable = false, .transition = false, .name = "Setzling"},
    {.symbol = 'v', .passable = false, .transition = false, .name = "Graue Spur"},
    {.symbol = 'n', .passable = false, .transition = false, .name = "Schild"},
    {.symbol = 'w', .passable = false, .transition = false, .name = "Schild"},
    {.symbol = 'c', .passable = false, .transition = false, .name = "Werkbank"},
    {.symbol = 'z', .passable = false, .transition = false, .name = "Totholzstapel"}};
const int tile_count = (int)(sizeof tiles / sizeof tiles[0]);
const TileDef *tile_def(char s) {
  for (int i = 0; i < tile_count; i++)
    if (tiles[i].symbol == s)
      return &tiles[i];
  return NULL;
}
char map_at(const Map *m, int x, int y) {
  return x < 0 || y < 0 || x >= m->width || y >= m->height ? '^'
                                                           : m->cells[y * m->width + x];
}
bool map_passable(const Map *m, int x, int y) {
  const TileDef *t = tile_def(map_at(m, x, y));
  return t && t->passable;
}
bool map_load(Map *m, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;
  Map next = {0};
  int w, h;
  char line[128];
  if (!fgets(line, sizeof line, f) || sscanf(line, "%d %d", &w, &h) != 2 || w < 1 ||
      h < 1 || w > MAP_LIMIT || h > MAP_LIMIT) {
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
