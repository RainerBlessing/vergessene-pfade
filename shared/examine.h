#ifndef SHARED_EXAMINE_H
#define SHARED_EXAMINE_H
/* Looking at things and using an item on them: the same on both versions.
 * Included by game.c after the helpers it leans on: matches, stone_at (shared),
 * stake_here (shared), use_point and examine_nothing, which each version has in
 * its own form. */

/* Order of the neighbours: north, east, south, west. */
static const int8_t neighbours[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

static const ExaminePoint *point_at(const Game *g, Coord x, Coord y) {
  char symbol = game_tile(g, g->map, x, y);
  for (Count i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_ITEM || !matches(g, p->needs, 0) || p->map != g->map)
      continue;
    if (p->only_after != OUT_NONE && p->only_after != g->outcome)
      continue; /* was der Stapel sagt, haengt am Ausgang */
    if (p->kind == POINT_STONE) {
      if (stone_at(g, x, y))
        return p;
      continue;
    }
    if (p->kind == POINT_AT ? p->x == (uint8_t)x && p->y == (uint8_t)y
                            : p->symbol == symbol)
      return p;
  }
  return NULL;
}
static const ExaminePoint *item_point_at(const Game *g, uint8_t item, char tile) {
  for (Count i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind != POINT_ITEM || p->item != item || !matches(g, p->needs, 0))
      continue;
    if (!p->symbol || (p->map == g->map && p->symbol == tile))
      return p;
  }
  return NULL;
}
static const ExaminePoint *point_for_item(const Game *g, uint8_t item) {
  const ExaminePoint *p =
      item_point_at(g, item, game_tile(g, g->map, g->x + g->dx, g->y + g->dy));
  if (p)
    return p;
  for (Count i = 0; i < 4; i++) {
    Coord x = (Coord)(g->x + neighbours[i][0]), y = (Coord)(g->y + neighbours[i][1]);
    p = item_point_at(g, item, game_tile(g, g->map, x, y));
    if (p && p->symbol) /* ohne Kachel hat schon der erste Versuch gegriffen */
      return p;
  }
  return NULL;
}

/* Blickrichtung zuerst, dann das eigene Feld, dann die uebrigen Nachbarn.
 * Der Blick entscheidet also weiter, was gemeint ist -- aber wer neben einer
 * Sache steht, findet sie auch, ohne sich erst dagegen zu druecken (#17). */
static void examine(Game *g) {
  if (stake_here(g))
    return;
  Coord fx = (Coord)(g->x + g->dx), fy = (Coord)(g->y + g->dy);
  const ExaminePoint *p = point_at(g, fx, fy);
  if (p) {
    use_point(g, p, game_tile(g, g->map, fx, fy));
    return;
  }
  p = point_at(g, g->x, g->y);
  if (p) {
    use_point(g, p, game_tile(g, g->map, g->x, g->y));
    return;
  }
  for (Count i = 0; i < 4; i++) {
    Coord x = (Coord)(g->x + neighbours[i][0]), y = (Coord)(g->y + neighbours[i][1]);
    if (x == fx && y == fy) /* schon angesehen */
      continue;
    p = point_at(g, x, y);
    if (p) {
      use_point(g, p, game_tile(g, g->map, x, y));
      return;
    }
  }
  examine_nothing(g, fx, fy);
}
#endif
