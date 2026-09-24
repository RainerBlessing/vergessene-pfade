#include "world.h"
#include "maps.h"

/* Screen codes and colors of the C64 text mode. The names are the same ones
 * the SDL build shows when there is nothing special about a tile. */
#define PASS TF_PASSABLE
static const TileDef tiles[] = {
    {'.', 46, 5, PASS, "Wiese"},
    {',', 44, 9, PASS, "Weg"},
    {'~', 102, 14, 0, "Bach"},
    {'T', 84, 5, 0, "Kiefer"},
    {'^', 160, 11, 0, "Fels"},
    {'#', 160, 9, 0, "Dorfmauer"},
    {'H', 160, 9, 0, "Hauswand"},
    /* Die Hausmarke sah aus wie jede andere Wand -- und daran haengt die ganze
     * Kintsugi-Kette (#15). Ein eingebranntes Zeichen, warm statt Mauerbraun. */
    {'M', 38 /* '&' */, 8, 0, "Hauswand"},
    {'R', 160, 2, 0, "Ziegeldach"},
    {'[', 160, 2, 0, "Dachkante"},
    {']', 160, 2, 0, "Dachkante"},
    {'_', 46, 11, PASS, "Boden"},
    {'+', 43, 7, PASS, "Tuer"},
    {'>', 62, 7, PASS | TF_TRANSITION, "Weg ins Dorf"},
    {'<', 60, 7, PASS | TF_TRANSITION, "Nordtor"},
    {'*', 42, 15, PASS, "Steinlaterne"},
    {'S', 83, 10, 0, "Sakura"},
    {'B', 66, 13, 0, "Bambus"},
    {'r', 102, 13, 0, "Reisfeld"},
    {'=', 61, 9, PASS, "Bruecke"},
    {'l', 12, 7, 0, "Papierlaterne"},
    {'s', 46, 12, PASS, "Kies"},
    {'G', 71, 15, 0, "Grenzstein"},
    {'x', 24, 9, 0, "Baumstumpf"},
    {'O', 79, 7, 0, "Schrein"},
    {'o', 15, 7, 0, "Opferstein"},
    {'Y', 89, 13, 0, "Alter Baum"},
    {'m', 45, 15, PASS, "Mulde"},
    {'d', 59, 9, PASS, "Aufgewuehlte Erde"},
    {'A', 65, 8, 0, "Zelt"},
    {'k', 11, 9, 0, "Werktisch"},
    {'W', 87, 9, 0, "Holzstapel"},
    {'h', 102, 5, PASS | TF_GUARDED, "Moosboden"},
    {'F', 70, 8, 0, "Fuchsbau"},
    {'f', 6, 8, 0, "Fuchsbau"},
    {'t', 39, 15, PASS, "Faehrte"},
    {'b', 2, 9, 0, "Regal"},
    {'q', 17, 9, 0, "Regal"},
    {'u', 21, 14, PASS, "Futon"},
    {'e', 5, 9, 0, "Leerer Bau"},
    {'n', 14, 7, 0, "Schild"},
    {'w', 23, 7, 0, "Schild"},
    {'c', 3, 9, 0, "Werkbank"},
    {'z', 26, 9, 0, "Totholzstapel"},
    {'p', 9, 9, PASS, "Grenzpfahl"},
};
static const uint8_t tile_count = (uint8_t)(sizeof tiles / sizeof tiles[0]);

/* A symbol is looked up per drawn character cell, so it may not be a search:
 * one byte per symbol costs 128 bytes and turns the lookup into an index. */
static uint8_t by_symbol[128];

void world_init(void) {
  for (uint8_t i = 0; i < 128; i++)
    by_symbol[i] = tile_count; /* "no such tile" */
  for (uint8_t i = 0; i < tile_count; i++)
    by_symbol[(uint8_t)tiles[i].symbol] = i;
}

const TileDef *tile_def(char symbol) {
  uint8_t i = (uint8_t)symbol < 128 ? by_symbol[(uint8_t)symbol] : tile_count;
  return i < tile_count ? &tiles[i] : 0;
}

const char *map_row(uint8_t map, uint8_t y) {
  const char *cells = map == MAP_VILLAGE ? village_cells : forest_cells;
  return cells + (int)y * map_width(map);
}

uint8_t map_width(uint8_t map) { return map == MAP_VILLAGE ? VILLAGE_W : FOREST_W; }
uint8_t map_height(uint8_t map) { return map == MAP_VILLAGE ? VILLAGE_H : FOREST_H; }

char map_at(uint8_t map, int8_t x, int8_t y) {
  uint8_t w = map_width(map), h = map_height(map);
  if (x < 0 || y < 0 || (uint8_t)x >= w || (uint8_t)y >= h)
    return '^'; /* off the map is rock: impassable and unremarkable */
  const char *cells = map == MAP_VILLAGE ? village_cells : forest_cells;
  return cells[(int)y * w + x];
}
