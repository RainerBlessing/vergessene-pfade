#ifndef SHARED_NIGHT_H
#define SHARED_NIGHT_H
/* The night at the inn: the question, the answers, and the morning, and what a
 * closing dialogue leads on to. The same on both versions. Included by game.c after the helpers it leans on: learn, emit,
 * open_scene, open_dialogue and enter_place. */

static void ask_about_the_night(Game *g) {
  g->state = GAME_PROMPT;
  g->dialogue = D_PROMPT_SLEEP;
  g->selection = 0;
  g->message[0] = 0;
}
static bool spend_the_night(Game *g) {
  if (g->phase != PHASE_BEFORE || g->outcome == OUT_NONE)
    return false;
  g->map = MAP_VILLAGE;
  g->x = INN_X;
  g->y = INN_Y;
  g->dx = 0;
  g->dy = 1;
  g->phase = PHASE_MORNING;
  g->place = NULL;
  enter_place(g); /* waking up, the inn names itself again */
  learn(g, OBS(OBS_MORNING), N_MORNING);
  emit(g, EV_PHASE, g->phase, 0);
  open_scene(g, "Im Gasthaus", D_SCENE_MORNING);
  return true;
}
static void prompt_action(Game *g, Action a) {
  if (a == ACT_UP || a == ACT_DOWN)
    g->selection = g->selection == 0 ? 1 : 0;
  if (a == ACT_CANCEL) { /* staying is the safe answer */
    g->state = GAME_EXPLORATION;
    return;
  }
  if (a != ACT_CONFIRM)
    return;
  g->state = GAME_EXPLORATION;
  if (g->selection != 0)
    return;
  if (!spend_the_night(g)) /* the forest is still unsettled */
    open_dialogue(g, -1, 'u', D_X_FUTON_AWAKE, OPEN_NOTHING);
}
/* Reading a dialogue to its end can lead on to the question: when it opens the
 * night, or when it was the futon explaining itself. Call it before the
 * dialogue's `opens` is cleared. */
static void ask_after_dialogue(Game *g, bool read_out, Count was) {
  if (read_out && g->phase == PHASE_BEFORE && (g->opens == OPEN_NIGHT || was == D_X_FUTON))
    ask_about_the_night(g);
}
/* What a dialogue sets in motion when it closes, however the player closed it: the
 * repair view, Daigo walking along. The night and the teaser need reading to the end
 * (see ask_after_dialogue). Call it before the dialogue's `opens` is cleared. */
static void opens_after_dialogue(Game *g, bool read_out, Count was) {
  if (g->opens == OPEN_MEND) {
    g->state = GAME_MEND;
    g->selection = 0;
  } else if (g->opens == OPEN_FOLLOW)
    g->daigo_follows = true;
  else if (g->opens == OPEN_TEASER && read_out)
    open_scene(g, "Die vergessenen Pfade", D_SCENE_TEASER);
  else
    ask_after_dialogue(g, read_out, was);
}
#endif
