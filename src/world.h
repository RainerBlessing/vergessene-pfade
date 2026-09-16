#ifndef WORLD_H
#define WORLD_H
#include <stdbool.h>
#include <stdint.h>
#define MAP_LIMIT 64
#define MAP_COUNT 2
typedef struct {
  char symbol;
  bool passable, transition;
  const char *name;
  bool guarded; /* stepping on it provokes the forest spirit */
} TileDef;
typedef struct {
  uint8_t width, height;
  char cells[MAP_LIMIT * MAP_LIMIT];
} Map;
extern const TileDef tiles[];
extern const int tile_count;
const TileDef *tile_def(char symbol);
bool map_load(Map *map, const char *path);
char map_at(const Map *map, int x, int y);
bool map_passable(const Map *map, int x, int y);
#endif
