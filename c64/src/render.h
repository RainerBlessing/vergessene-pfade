#ifndef RENDER_H
#define RENDER_H
#include "game.h"

/* What the last draw of the map area remembered; der Aufrufer haelt ihn
 * zwischen zwei Bildern. Ein genullter Cache sagt "noch nichts gezeichnet",
 * also zeichnet das erste render() die Karte vollstaendig -- darum genuegt
 * `RenderCache cache = {0};` und niemand muss an ein Feld denken. Tafeln, die
 * die Karte ueberdecken, rufen cache_invalidate(); nur dort wird `drawn`
 * zurueckgenommen. */
typedef struct {
  bool drawn; /* die Karte steht so, wie die Felder hier es sagen */
  uint8_t map, x, y, outcome, stone_x, stone_y, staked;
  Obs obs;
  int8_t cx, cy;
} RenderCache;

void render(const Game *g, RenderCache *cache);
void cache_invalidate(RenderCache *cache);
#endif
