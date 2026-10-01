#ifndef SHARED_STONE_H
#define SHARED_STONE_H
/* The boundary stone the loggers moved, and pushing it back: the same on both
 * versions. Included by game.c after the helpers it leans on: matches,
 * plain_tile (an ordinary tile the stone, or a step back, may land on), learn, finish, open_scene
 * and emit. stone_at and reset_stone are declared earlier in game.c, because the
 * examine code above this point asks for them. */
bool game_can_push(const Game *g) {
  /* Die Ausgaenge schliessen sich aus. Der Kompromiss ist erst mit dem dritten
   * Pfahl entschieden -- angefangen ist er aber schon vorher, und dann bleibt
   * der Stein liegen, wo er liegt. */
  return matches(g, OBS(OBS_STONE_DRAGGED) | OBS(OBS_STONE_HOLLOW), 0) &&
         g->outcome == OUT_NONE && !g->daigo_follows && g->staked == 0;
}
static bool stone_at(const Game *g, Coord x, Coord y) {
  return g->map == MAP_FOREST && x == g->stone_x && y == g->stone_y;
}
/* Rolling the stuck stone back to where the drag marks start. It is heard like a
 * push. */
static void reset_stone(Game *g) {
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  g->obs &= ~OBS(OBS_STONE_MOVED);
  emit(g, EV_STONE_PUSH, g->stone_x, g->stone_y);
}
static bool push_stone(Game *g, Coord dx, Coord dy) {
  Coord tx = (Coord)(g->stone_x + dx), ty = (Coord)(g->stone_y + dy);
  if (!game_can_push(g) || !plain_tile(tile_def(game_tile(g, MAP_FOREST, tx, ty))) ||
      game_npc_at(g, tx, ty) >= 0)
    return false;
  g->stone_x = tx;
  g->stone_y = ty;
  bool settled = tx == STONE_HOLLOW_X && ty == STONE_HOLLOW_Y;
  emit(g, EV_STONE_PUSH, tx, ty);
  if (settled)
    g->obs &= ~OBS(OBS_STONE_MOVED);
  else
    g->obs |= OBS(OBS_STONE_MOVED);
  if (settled) {
    g->mood = MOOD_CALM;
    learn(g, 0, N_BOUNDARY);
    finish(g, OUT_BOUNDARY);
    open_scene(g, "Die alte Grenze", D_SCENE_BOUNDARY);
  }
  return true;
}
#endif
