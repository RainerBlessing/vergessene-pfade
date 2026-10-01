#ifndef SHARED_STAKE_H
#define SHARED_STAKE_H
/* Driving in the stakes of the new boundary together with Daigo: the same on
 * both versions. Included by game.c after the helpers it leans on: learn,
 * finish, open_scene and emit. */
static bool stake_here(Game *g) {
  Coord dx = (Coord)(g->daigo_x - g->x), dy = (Coord)(g->daigo_y - g->y);
  if (!g->daigo_follows || g->map != MAP_FOREST || dx * dx + dy * dy > 1)
    return false; /* the two of them drive it in together */
  if (g->outcome != OUT_NONE)
    return false; /* was entschieden ist, ist entschieden */
  for (Count i = 0; i < STAKE_COUNT; i++) {
    if (stakes[i].x != (uint8_t)g->x || stakes[i].y != (uint8_t)g->y ||
        (g->staked & (1u << i)))
      continue;
    g->staked |= (uint8_t)(1u << i);
    Count count = 0;
    for (Count k = 0; k < STAKE_COUNT; k++)
      count = (Count)(count + ((g->staked >> k) & 1u));
    emit(g, EV_STAKE, count, 0);
    if (count == STAKE_COUNT) {
      g->daigo_follows = false;
      g->daigo_x = (Coord)npcs[NPC_DAIGO].x;
      g->daigo_y = (Coord)npcs[NPC_DAIGO].y;
      learn(g, 0, N_MEND);
      finish(g, OUT_MEND);
      open_scene(g, "Die neue Grenze", D_SCENE_MEND);
    } else
      open_scene(g, "Entlang der Spuren", D_STAKE_SET);
    return true;
  }
  return false;
}
#endif
