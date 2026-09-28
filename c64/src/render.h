#ifndef RENDER_H
#define RENDER_H
#include "game.h"

/* What the last draw of the map area remembered. A fresh cache has the map
 * marked stale, so the first render() draws it in full; the caller keeps it
 * between renders. Panels that cover the map call cache_invalidate(), which is
 * the only place that may set stale. */
typedef struct {
  bool stale; /* something was drawn over the map area */
  uint8_t map, x, y, outcome, stone_x, stone_y, staked;
  Obs obs;
  int8_t cx, cy;
} RenderCache;

void render(const Game *g, RenderCache *cache);
void cache_invalidate(RenderCache *cache);
#endif
