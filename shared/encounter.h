#ifndef SHARED_ENCOUNTER_H
#define SHARED_ENCOUNTER_H
/* The meeting with the kami: what can be chosen, and what a choice does. The same
 * on both versions. Included by game.c after the helpers it leans on, which each
 * version has in its own form: carries (does the player hold this item),
 * encounter_say (put a line of the encounter on the panel), take_item,
 * plain_tile, select_move, learn, emit, can_enter and fight_round (the blows
 * themselves differ in cost on the C64, see fight.h). Each must be defined or at
 * least declared above the include; plain_tile and can_enter are only declared
 * there, so moving the include up is fine as long as those declarations stay
 * above it. */

static const EncounterOffer *offer_at_hand(const Game *g) {
  for (Count i = 0; i < encounter_offer_count; i++)
    if (carries(g, encounter_offers[i].item))
      return &encounter_offers[i];
  return NULL;
}
Count game_encounter_options(const Game *g, Count *out) {
  Count n = 0;
  for (Count i = 0; i < encounter_option_count; i++) {
    const EncounterOption *o = &encounter_options[i];
    if (o->when != OPT_BOTH && (o->when == OPT_FIGHT) != (g->fighting != 0))
      continue;
    if (o->action == ENC_OFFER && !offer_at_hand(g))
      continue;
    if (o->action == ENC_HEAL && !carries(g, ITEM_HERB))
      continue;
    out[n++] = i;
  }
  return n;
}
static void step_back(Game *g) {
  Coord bx = (Coord)(g->x - g->dx), by = (Coord)(g->y - g->dy);
  if (can_enter(g, bx, by) && plain_tile(tile_def(game_tile(g, g->map, bx, by)))) {
    g->x = bx;
    g->y = by;
  }
}
/* The spirit rises from the grove; its mood is remembered between encounters. */
static void begin_encounter(Game *g) {
  g->state = GAME_ENCOUNTER;
  g->selection = 0;
  learn(g, OBS(OBS_KAMI_SEEN), N_KAMI);
  g->message[0] = 0;
  encounter_say(g, D_ENC_APPEAR);
  emit(g, EV_ENCOUNTER, g->mood, 0);
}
static void encounter_action(Game *g, Action a) {
  Count options[ENCOUNTER_OPTION_LIMIT];
  Count count = game_encounter_options(g, options);
  if (count == 0)
    return; /* nothing to choose from */
  select_move(g, a, count);
  if (a == ACT_CANCEL) /* Escape highlights retreating, it does not do it */
    for (Count i = 0; i < count; i++)
      if (encounter_options[options[i]].action == ENC_RETREAT)
        g->selection = i;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  Count action = encounter_options[options[g->selection]].action;
  g->message[0] = 0; /* last round's numbers belong to the last round */
  const EncounterOffer *offer = action == ENC_OFFER ? offer_at_hand(g) : NULL;
  Count before = g->mood;
  g->mood = offer ? offer->result : encounter_transitions[action][before];
  if (offer) {
    if (offer->takes != ITEM_NONE) /* die Schale bleibt im Moos stehen */
      take_item(g, offer->takes);
    learn(g, offer->grants, offer->note);
    encounter_say(g, offer->dialogue);
  } else
    encounter_say(g, encounter_lines[action][before]);
  emit(g, EV_ENCOUNTER_ACTION, action, g->mood);
  if (action == ENC_ATTACK || action == ENC_HEAL) {
    fight_round(g, action == ENC_HEAL);
    g->selection = 0;
    return;
  }
  if (action == ENC_RETREAT) {
    step_back(g);
    g->fighting = false;
    g->state = GAME_EXPLORATION;
  }
}
#endif
