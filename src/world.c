#include "world.h"
#include <stdio.h>
#include <string.h>
const TileDef tiles[] = {
    {'.', true, false, "Wiese"},       {',', true, false, "Weg"},
    {'~', false, false, "Wasser"},     {'T', false, false, "Kiefer"},
    {'^', false, false, "Fels"},       {'#', false, false, "Dorfmauer"},
    {'H', false, false, "Hauswand"},   {'_', true, false, "Boden"},
    {'+', true, false, "Tuer"},        {'>', true, true, "Dorftor"},
    {'<', true, true, "Aussenwelt"},   {'*', true, false, "Steinlaterne"},
    {'S', false, false, "Sakura"},     {'B', false, false, "Bambus"},
    {'r', false, false, "Reisfeld"},   {'=', true, false, "Bruecke"},
    {'R', false, false, "Ziegeldach"}, {'l', false, false, "Papierlaterne"},
    {'s', true, false, "Kies"},        {'[', false, false, "Dachkante"},
    {']', false, false, "Dachkante"}};
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
