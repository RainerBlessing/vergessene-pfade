#ifndef SHARED_MEND_H
#define SHARED_MEND_H
/* Setting the pieces of the bowl: the same on both versions. Included by game.c
 * after the helpers it leans on, which each version has in its own form:
 * select_move, show, take_item, learn, open_scene and emit. */
static void mend_action(Game *g, Action a) {
  Count pieces[MEND_PIECES];
  Count count = game_mend_pieces(g, pieces);
  if (a == ACT_CANCEL) {
    g->state = GAME_EXPLORATION;
    return;
  }
  if (count == 0)
    return;
  select_move(g, a, count);
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  bool fits = pieces[g->selection] == g->mend_placed;
  if (fits)
    g->mend_placed++;
  emit(g, EV_MEND, g->mend_placed, fits);
  g->selection = 0;
  if (!fits) { /* a piece that does not fit costs nothing */
    show(g, D_MEND_WRONG);
    return;
  }
  g->message[0] = 0;
  if (g->mend_placed == MEND_PIECES) {
    take_item(g, ITEM_SHARDS);
    learn(g, OBS(OBS_BOWL_DRYING), N_MENDED);
    open_scene(g, "In Orihas Werkstatt", D_MEND_DONE);
  }
}
#endif
