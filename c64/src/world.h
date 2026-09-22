#ifndef WORLD_H
#define WORLD_H
#include <stdbool.h>
#include <stdint.h>

enum { MAP_VILLAGE, MAP_FOREST, MAP_COUNT };

/* Flags of a tile; everything else about it is its look. */
#define TF_PASSABLE 0x01
#define TF_TRANSITION 0x02
#define TF_GUARDED 0x04 /* stepping on it provokes the forest spirit */

typedef struct {
  char symbol;
  uint8_t screen; /* PETSCII screen code */
  uint8_t color;
  uint8_t flags;
  const char *name;
} TileDef;

/* Builds the symbol -> tile table; call once before tile_def(). */
void world_init(void);
const TileDef *tile_def(char symbol);
/* The row as it was drawn on the map, without any overrides. */
const char *map_row(uint8_t map, uint8_t y);
uint8_t map_width(uint8_t map);
uint8_t map_height(uint8_t map);
char map_at(uint8_t map, int8_t x, int8_t y);
#endif
