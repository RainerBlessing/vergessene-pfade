#include "game.h"
#include <stdio.h>
#include <string.h>
bool game_init(Game *g, const char *assets) {
  memset(g, 0, sizeof *g);
  g->map = MAP_FOREST;
  g->x = 24;
  g->y = 19;
  g->dy = -1;
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  g->daigo_x = npcs[NPC_DAIGO].x;
  g->daigo_y = npcs[NPC_DAIGO].y;
  g->npc = SPEAKER_SCENE;
  g->scene = "Unterwegs";
  g->dialogue = D_SCENE_ARRIVAL;
  g->state = GAME_TITLE; /* the arrival scene waits behind the title page */
  g->player = (Player){.hp = 24, .max_hp = 24, .attack = 8, .defense = 2};
  char path[1024];
  snprintf(path, sizeof path, "%s/maps/village.map", assets);
  if (!map_load(&g->maps[MAP_VILLAGE], path))
    return false;
  snprintf(path, sizeof path, "%s/maps/forest.map", assets);
  return map_load(&g->maps[MAP_FOREST], path);
}
void game_npc_pos(const Game *g, int npc, int *x, int *y) {
  *x = npc == NPC_DAIGO ? g->daigo_x : npcs[npc].x;
  *y = npc == NPC_DAIGO ? g->daigo_y : npcs[npc].y;
}
int game_npc_at(const Game *g, int x, int y) {
  for (int i = 0; i < NPC_COUNT; i++) {
    int nx, ny;
    game_npc_pos(g, i, &nx, &ny);
    if (npcs[i].map == g->map && nx == x && ny == y)
      return i;
  }
  return -1;
}
bool game_knows(const Game *g, ObsId o) { return (g->obs & OBS(o)) != 0; }
static bool matches(const Game *g, Obs needs, Obs forbids) {
  return (g->obs & needs) == needs && !(g->obs & forbids);
}
static bool fits_now(const Game *g, Obs needs, Outcome outcome, Phase phase) {
  return (g->obs & needs) == needs && (outcome == OUT_ANY || outcome == g->outcome) &&
         (phase == PHASE_ANY || phase == g->phase);
}
char game_tile(const Game *g, int map, int x, int y) {
  if (map == MAP_FOREST && x == g->stone_x && y == g->stone_y)
    return 'G';
  if (map == MAP_FOREST)
    for (int i = 0; i < STAKE_COUNT; i++)
      if ((g->staked & (1u << i)) && stakes[i].x == x && stakes[i].y == y)
        return 'p';
  /* What an outcome changed comes first: it is the newer state of the world. */
  for (int i = 0; i < outcome_change_count; i++) {
    const TileOverride *o = &outcome_changes[i];
    if (o->map == map && o->x == x && o->y == y &&
        fits_now(g, o->needs, o->outcome, o->phase))
      return o->symbol;
  }
  for (int i = 0; i < tile_override_count; i++) {
    const TileOverride *o = &tile_overrides[i];
    if (o->map == map && o->x == x && o->y == y &&
        fits_now(g, o->needs, o->outcome, o->phase))
      return o->symbol;
  }
  return map_at(&g->maps[map], x, y);
}
bool game_passable(const Game *g, int map, int x, int y) {
  const TileDef *t = tile_def(game_tile(g, map, x, y));
  return t && t->passable;
}
int game_owned_items(const Game *g, ItemId *out) {
  int n = 0;
  for (int i = ITEM_NONE + 1; i < ITEM_COUNT; i++)
    if (g->player.inventory.quantities[i])
      out[n++] = (ItemId)i;
  return n;
}
int game_take_events(Game *g, GameEvent *out, int max) {
  int n = g->event_count < max ? g->event_count : max;
  memcpy(out, g->events, (size_t)n * sizeof *out);
  memmove(g->events, g->events + n, (size_t)(g->event_count - n) * sizeof *out);
  g->event_count -= n;
  return n;
}
static void emit(Game *g, EventType type, int a, int b) {
  if (g->event_count >= EVENT_LIMIT) {
    g->events_dropped++;
    return;
  }
  g->events[g->event_count++] = (GameEvent){type, g->map, g->x, g->y, a, b};
}
static void learn(Game *g, Obs grants, NoteId note) {
  for (int o = 0; o < OBS_COUNT; o++)
    if ((grants & OBS(o)) && !(g->obs & OBS(o))) {
      g->obs |= OBS(o);
      emit(g, EV_OBSERVE, o, 0);
    }
  if (note == NOTE_NONE)
    return;
  for (int i = 0; i < g->note_count; i++)
    if (g->notes[i] == note)
      return;
  if (g->note_count < NOTE_LIMIT) {
    g->notes[g->note_count++] = note;
    g->note_ticks = NOTE_TICKS; /* only a new entry announces itself */
  }
}
/* Keeps g->message: a scene page left empty shows it (see dialogue_panel). */
static void open_scene(Game *g, const char *title, DialogueId dialogue) {
  g->npc = SPEAKER_SCENE;
  g->scene = title;
  g->examined = 0;
  g->dialogue = dialogue;
  g->page = 0;
  g->state = GAME_DIALOGUE;
}
static void open_dialogue(Game *g, int npc, char examined, DialogueId dialogue) {
  g->opens = OPEN_NOTHING;
  g->npc = npc;
  g->examined = examined;
  g->dialogue = dialogue;
  g->page = 0;
  g->message[0] = 0;
  g->state = GAME_DIALOGUE;
}
int game_mend_pieces(const Game *g, int *out) {
  int n = 0;
  for (int i = 0; i < MEND_PIECES; i++)
    if (mend_display[i] >= g->mend_placed)
      out[n++] = mend_display[i];
  return n;
}
static void talk(Game *g, int npc) {
  for (int i = 0; i < dialogue_rule_count; i++) {
    const DialogueRule *r = &dialogue_rules[i];
    if ((int)r->npc != npc || !matches(g, r->needs, r->forbids))
      continue;
    if (!fits_now(g, 0, r->outcome, r->phase))
      continue;
    /* Do not hand over something the player still carries: the next rule speaks. */
    if (r->gives != ITEM_NONE && g->player.inventory.quantities[r->gives])
      continue;
    if (r->gives != ITEM_NONE)
      inventory_add(&g->player.inventory, r->gives, 1);
    learn(g, r->grants, r->note);
    emit(g, EV_NPC_TALK, npc, r->dialogue);
    open_dialogue(g, npc, 0, r->dialogue);
    g->opens = r->opens; /* after opening: open_dialogue clears it */
    return;
  }
}
static bool stone_at(const Game *g, int x, int y) {
  return g->map == MAP_FOREST && x == g->stone_x && y == g->stone_y;
}
static const ExaminePoint *point_at(const Game *g, int x, int y) {
  char symbol = game_tile(g, g->map, x, y);
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_ITEM || !matches(g, p->needs, 0))
      continue;
    if (p->kind == POINT_STONE) {
      if (stone_at(g, x, y))
        return p;
      continue;
    }
    if (p->map != g->map)
      continue;
    if (p->kind == POINT_AT ? p->x == x && p->y == y : p->symbol == symbol)
      return p;
  }
  return NULL;
}
static const ExaminePoint *point_for_item(const Game *g, ItemId item) {
  char facing = game_tile(g, g->map, g->x + g->dx, g->y + g->dy);
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind != POINT_ITEM || p->item != item || !matches(g, p->needs, 0))
      continue;
    if (!p->symbol || (p->map == g->map && p->symbol == facing))
      return p;
  }
  return NULL;
}
/* Rolling the stuck stone back to where the drag marks start. */
static void reset_stone(Game *g) {
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  g->obs &= ~OBS(OBS_STONE_MOVED);
  emit(g, EV_STONE_PUSH, g->stone_x, g->stone_y);
}
static void use_point(Game *g, const ExaminePoint *p, char examined) {
  if (p->kind == POINT_STONE && p->dialogue == D_X_STONE_STUCK)
    reset_stone(g);
  /* Reading about the visitor in the morning closes the slice, once - but only
   * after the inscription itself has been read. */
  bool closes =
      p->note == N_VISITOR && g->phase == PHASE_MORNING && !game_knows(g, OBS_TEASED);
  if (p->takes != ITEM_NONE)
    inventory_remove(&g->player.inventory, p->takes, 1);
  if (p->gives != ITEM_NONE)
    inventory_add(&g->player.inventory, p->gives, 1);
  learn(g, p->grants, p->note);
  emit(g, EV_EXAMINE, examined, p->dialogue);
  open_dialogue(g, -1, examined, p->dialogue);
  if (closes) {
    learn(g, OBS(OBS_TEASED), NOTE_NONE);
    g->opens = OPEN_TEASER;
  }
}
/* Repeated presses at the same target are counted, not logged again. */
static void examine_nothing(Game *g, int x, int y) {
  NothingTarget *last = &g->last_nothing;
  char symbol = game_tile(g, g->map, x, y);
  const TileDef *tile = tile_def(symbol);
  snprintf(g->message, sizeof g->message, "%s: nichts Besonderes.",
           tile ? tile->name : "Dort");
  if (last->valid && last->map == g->map && last->x == x && last->y == y &&
      last->dx == g->dx && last->dy == g->dy) {
    last->repeat++;
    return;
  }
  emit(g, EV_EXAMINE_NOTHING, symbol, last->valid ? last->repeat : 0);
  *last = (NothingTarget){true, g->map, x, y, g->dx, g->dy, 0};
}
static bool stake_here(Game *g);
/* Facing tile first, then the tile underfoot (passable points cannot be faced). */
static void examine(Game *g) {
  if (stake_here(g))
    return;
  int fx = g->x + g->dx, fy = g->y + g->dy;
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
  examine_nothing(g, fx, fy);
}
static void inventory_action(Game *g, Action a) {
  ItemId owned[ITEM_COUNT];
  int count = game_owned_items(g, owned);
  if (a == ACT_CANCEL || a == ACT_INVENTORY) {
    g->state = GAME_EXPLORATION;
    return;
  }
  if (count == 0)
    return;
  if (a == ACT_UP)
    g->selection = (g->selection + count - 1) % count;
  if (a == ACT_DOWN)
    g->selection = (g->selection + 1) % count;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  ItemId item = owned[g->selection];
  const ExaminePoint *p = point_for_item(g, item);
  if (p) {
    emit(g, EV_ITEM_USE, item, p->dialogue);
    use_point(g, p, p->symbol);
    return;
  }
  bool healed = item == ITEM_HERB && player_heal(&g->player);
  emit(g, EV_ITEM_USE, item, D_NONE);
  snprintf(g->message, sizeof g->message, "%s",
           healed              ? "Das Kraut lindert deine Wunden."
           : item == ITEM_HERB ? "Du bist unverletzt."
                               : "Damit kannst du hier nichts tun.");
}
static void notebook_action(Game *g, Action a) {
  int last = g->note_count > NOTES_PER_PAGE ? g->note_count - NOTES_PER_PAGE : 0;
  if (a == ACT_CANCEL || a == ACT_CONFIRM)
    g->state = GAME_EXPLORATION;
  else if (a == ACT_UP && g->scroll > 0)
    g->scroll--;
  else if (a == ACT_DOWN && g->scroll < last)
    g->scroll++;
}
/* The follower steps aside (they swap), everyone else blocks. */
static bool blocking_npc(const Game *g, int x, int y) {
  int npc = game_npc_at(g, x, y);
  return npc >= 0 && !(npc == NPC_DAIGO && g->daigo_follows);
}
static bool can_enter(const Game *g, int x, int y) {
  return !blocking_npc(g, x, y) && game_passable(g, g->map, x, y);
}
/* Guarded ground pushes the player one step away, onto free, safe ground. */
static void step_back(Game *g) {
  int bx = g->x - g->dx, by = g->y - g->dy;
  const TileDef *back = tile_def(game_tile(g, g->map, bx, by));
  if (can_enter(g, bx, by) && !back->guarded && !back->transition) {
    g->x = bx;
    g->y = by;
  }
}
static bool option_now(const Game *g, const EncounterOption *o) {
  if (!matches(g, o->needs, 0))
    return false;
  if (o->when != OPT_BOTH && (o->when == OPT_FIGHT) != g->fighting)
    return false;
  return true;
}
static const EncounterOffer *offer_at_hand(const Game *g) {
  for (int i = 0; i < encounter_offer_count; i++) {
    const EncounterOffer *o = &encounter_offers[i];
    if (g->player.inventory.quantities[o->item] && matches(g, o->needs, 0))
      return o;
  }
  return NULL;
}
ItemId game_encounter_offer(const Game *g) {
  const EncounterOffer *o = offer_at_hand(g);
  return o ? o->item : ITEM_NONE;
}
int game_encounter_options(const Game *g, int *out) {
  int n = 0;
  for (int i = 0; i < encounter_option_count; i++) {
    const EncounterOption *o = &encounter_options[i];
    if (!option_now(g, o) || (o->action == ENC_OFFER && !offer_at_hand(g)) ||
        (o->action == ENC_HEAL && !g->player.inventory.quantities[ITEM_HERB]))
      continue;
    out[n++] = i;
  }
  return n;
}
static void show(Game *g, DialogueId line) {
  const char *text = line > D_NONE ? dialogues[line].pages[0] : NULL;
  snprintf(g->message, sizeof g->message, "%s", text ? text : "");
}
static void finish(Game *g, Outcome outcome) {
  if (g->outcome != OUT_NONE)
    return;
  g->outcome = outcome;
  emit(g, EV_OUTCOME, outcome, 0);
}
/* The name of the room the player is standing in, if it has one. */
static const char *place_at(const Game *g, int map, int x, int y) {
  for (int i = 0; i < place_count; i++) {
    const Place *p = &places[i];
    if (p->map != map || !matches(g, p->needs, 0))
      continue;
    if (x >= p->x && x < p->x + p->width && y >= p->y && y < p->y + p->height)
      return p->name;
  }
  return NULL;
}
static void enter_place(Game *g) {
  const char *place = place_at(g, g->map, g->x, g->y);
  if (!place || place == g->place)
    return;
  g->place = place;
  g->place_ticks = PLACE_TICKS;
}
/* The one way into the next morning, whether the night was offered by Sumi or
 * taken at the futon. It happens once, and only once something is settled. */
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
static void ask_about_the_night(Game *g) {
  g->state = GAME_PROMPT;
  g->dialogue = D_PROMPT_SLEEP;
  g->selection = 0;
  g->message[0] = 0;
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
    open_dialogue(g, -1, 'u', D_X_FUTON_AWAKE);
}
/* One exchange of blows. The fight continues until someone falls or the player
 * steps back; the spirit keeps its wounds until it wins. */
static void fight_round(Game *g, bool herb) {
  if (!g->fighting) {
    g->fighting = true;
    if (g->combat.hp <= 0) /* a paused fight keeps the spirit's wounds */
      combat_begin(&g->combat);
  }
  combat_turn(&g->combat, &g->player, g->mood == MOOD_ANGRY ? 1 : 0, herb, g->message,
              sizeof g->message);
  if (g->combat.won) {
    g->fighting = false;
    learn(g, 0, N_FOUGHT);
    finish(g, OUT_FIGHT);
    open_scene(g, "Am Rand des Hains", D_ENC_VICTORY);
  } else if (g->combat.lost) {
    g->fighting = false;
    /* Carried home through the gate, facing into the village. */
    for (int i = 0; i < TRANSITION_COUNT; i++)
      if (transitions[i].to_map == MAP_VILLAGE) {
        g->map = MAP_VILLAGE;
        g->x = transitions[i].to_x;
        g->y = transitions[i].to_y;
        break;
      }
    g->dx = 0;
    g->dy = 1;
    g->player.hp = g->player.max_hp;
    combat_begin(&g->combat); /* the spirit recovers as well */
    open_scene(g, "Kiriyama, spaeter", D_ENC_DEFEAT);
  }
}
/* The spirit rises from the grove; its mood is remembered between encounters. */
static void begin_encounter(Game *g) {
  g->state = GAME_ENCOUNTER;
  g->selection = 0;
  learn(g, OBS(OBS_KAMI_SEEN), N_KAMI);
  show(g, D_ENC_APPEAR);
  emit(g, EV_ENCOUNTER, g->mood, 0);
}
/* Kintsugi: each piece is set into the gap it belongs to. A piece that does not
 * fit costs nothing; the seams stay visible. */
static void mend_action(Game *g, Action a) {
  int pieces[MEND_PIECES];
  int count = game_mend_pieces(g, pieces);
  if (a == ACT_CANCEL) {
    g->state = GAME_EXPLORATION;
    return;
  }
  if (count == 0)
    return;
  if (a == ACT_UP)
    g->selection = (g->selection + count - 1) % count;
  if (a == ACT_DOWN)
    g->selection = (g->selection + 1) % count;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  bool fits = pieces[g->selection] == g->mend_placed;
  if (fits)
    g->mend_placed++;
  emit(g, EV_MEND, g->mend_placed, fits);
  g->selection = 0;
  if (!fits) {
    show(g, D_MEND_WRONG);
    return;
  }
  g->message[0] = 0;
  if (g->mend_placed == MEND_PIECES) {
    inventory_remove(&g->player.inventory, ITEM_SHARDS, 1);
    learn(g, OBS(OBS_BOWL_DRYING), N_MENDED);
    open_scene(g, "In Orihas Werkstatt", D_MEND_DONE);
  }
}
/* Driving in a stake: only where the tracks run, and only with Daigo there. */
static bool stake_here(Game *g) {
  int near = (g->daigo_x - g->x) * (g->daigo_x - g->x) +
             (g->daigo_y - g->y) * (g->daigo_y - g->y);
  if (!g->daigo_follows || g->map != MAP_FOREST || near > 1)
    return false; /* the two of them drive it in together */
  for (int i = 0; i < STAKE_COUNT; i++) {
    if (stakes[i].x != g->x || stakes[i].y != g->y || (g->staked & (1u << i)))
      continue;
    g->staked |= (uint8_t)(1u << i);
    int count = 0;
    for (int k = 0; k < STAKE_COUNT; k++)
      count += (g->staked >> k) & 1u;
    emit(g, EV_STAKE, count, 0);
    if (count == STAKE_COUNT) {
      g->daigo_follows = false;
      g->daigo_x = npcs[NPC_DAIGO].x;
      g->daigo_y = npcs[NPC_DAIGO].y;
      learn(g, 0, N_MEND);
      finish(g, OUT_MEND);
      open_scene(g, "Die neue Grenze", D_SCENE_MEND);
    } else
      open_scene(g, "Entlang der Spuren", D_STAKE_SET);
    return true;
  }
  return false;
}
static void encounter_action(Game *g, Action a) {
  int options[ENCOUNTER_OPTION_LIMIT];
  int count = game_encounter_options(g, options);
  if (a == ACT_UP)
    g->selection = (g->selection + count - 1) % count;
  if (a == ACT_DOWN)
    g->selection = (g->selection + 1) % count;
  if (a == ACT_CANCEL) /* Escape highlights retreating, it does not do it */
    for (int i = 0; i < count; i++)
      if (encounter_options[options[i]].action == ENC_RETREAT)
        g->selection = i;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  EncounterAction action = encounter_options[options[g->selection]].action;
  const EncounterOffer *offer = action == ENC_OFFER ? offer_at_hand(g) : NULL;
  Mood before = g->mood;
  g->mood = offer ? offer->result : encounter_transitions[action][before];
  if (offer) {
    learn(g, offer->grants, offer->note);
    show(g, offer->dialogue);
  } else
    show(g, encounter_lines[action][before]);
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
bool game_can_push(const Game *g) {
  return matches(g, OBS(OBS_STONE_DRAGGED) | OBS(OBS_STONE_HOLLOW), 0) &&
         g->outcome == OUT_NONE;
}
/* Pushing the stone one tile. Nothing here knows why it matters: the inscription
 * and the empty hollow say that, and the player draws the line. */
static bool push_stone(Game *g, int dx, int dy) {
  int tx = g->stone_x + dx, ty = g->stone_y + dy;
  const TileDef *target = tile_def(game_tile(g, MAP_FOREST, tx, ty));
  if (!game_can_push(g) || !target || !target->passable || target->guarded ||
      target->transition || game_npc_at(g, tx, ty) >= 0)
    return false;
  g->stone_x = tx;
  g->stone_y = ty;
  bool settled = tx == STONE_HOLLOW_X && ty == STONE_HOLLOW_Y;
  if (settled)
    g->obs &= ~OBS(OBS_STONE_MOVED);
  else
    g->obs |= OBS(OBS_STONE_MOVED);
  emit(g, EV_STONE_PUSH, g->stone_x, g->stone_y);
  if (settled) {
    g->mood = MOOD_CALM;
    learn(g, 0, N_BOUNDARY);
    finish(g, OUT_BOUNDARY);
    open_scene(g, "Die alte Grenze", D_SCENE_BOUNDARY);
  }
  return true;
}
static void move(Game *g, int dx, int dy) {
  g->dx = dx;
  g->dy = dy;
  if (stone_at(g, g->x + dx, g->y + dy)) {
    if (push_stone(g, dx, dy)) {
      g->x += dx;
      g->y += dy;
    }
    return;
  }
  if (!can_enter(g, g->x + dx, g->y + dy))
    return;
  const TileDef *tile = tile_def(game_tile(g, g->map, g->x + dx, g->y + dy));
  if (tile->guarded && g->outcome == OUT_NONE) { /* a settled grove lets you pass */
    begin_encounter(g);
    return;
  }
  int from_x = g->x, from_y = g->y;
  g->x += dx;
  g->y += dy;
  g->steps++;
  if (!place_at(g, g->map, g->x, g->y))
    g->place = NULL; /* outside again: the next room may repeat its name */
  else
    enter_place(g);
  if (g->daigo_follows) { /* he walks in your footsteps */
    g->daigo_x = from_x;
    g->daigo_y = from_y;
  }
  if (!tile->transition)
    return;
  for (int i = 0; i < TRANSITION_COUNT; i++) {
    const Transition *t = &transitions[i];
    if (g->map == t->map && g->x == t->x && g->y == t->y) {
      bool from_forest = g->map == MAP_FOREST;
      g->map = t->to_map;
      g->x = t->to_x;
      g->y = t->to_y;
      /* The lacquer needs rest: it has dried by the time you are back. */
      if (from_forest && g->map == MAP_VILLAGE && game_knows(g, OBS_BOWL_DRYING))
        learn(g, OBS(OBS_BOWL_READY), NOTE_NONE);
      if (g->daigo_follows) { /* he stays in the forest, at his camp */
        g->daigo_follows = false;
        g->daigo_x = npcs[NPC_DAIGO].x;
        g->daigo_y = npcs[NPC_DAIGO].y;
      }
      g->place = NULL; /* the name belongs to the room actually entered */
      enter_place(g);
      return;
    }
  }
}
void game_action(Game *g, Action a) {
  if (g->place_ticks > 0) /* the room's name fades after a few moves */
    g->place_ticks--;
  if (g->note_ticks > 0) /* so does the notice about a new entry */
    g->note_ticks--;
  if (a == ACT_DEBUG) {
    g->debug = !g->debug;
    return;
  }
  if (a == ACT_COLLISION) {
    g->collision = !g->collision;
    return;
  }
  switch (g->state) {
  case GAME_TITLE:
    if (a == ACT_CONFIRM)
      g->state = GAME_DIALOGUE;
    return;
  case GAME_NOTEBOOK:
    notebook_action(g, a);
    return;
  case GAME_DIALOGUE: {
    if (a != ACT_CANCEL &&
        !(a == ACT_CONFIRM && ++g->page >= dialogues[g->dialogue].count))
      return;
    /* Escape closes and nothing more; reading to the end can lead on. */
    bool read_out = a == ACT_CONFIRM;
    DialogueId was = g->dialogue;
    g->state = GAME_EXPLORATION;
    g->message[0] = 0; /* the scene's echo of the last round ends with it */
    if (g->opens == OPEN_MEND) {
      g->state = GAME_MEND;
      g->selection = 0;
    } else if (g->opens == OPEN_FOLLOW)
      g->daigo_follows = true;
    else if (g->opens == OPEN_TEASER && read_out)
      open_scene(g, "Die vergessenen Pfade", D_SCENE_TEASER);
    else if (g->opens == OPEN_NIGHT && read_out && g->phase == PHASE_BEFORE)
      ask_about_the_night(g);
    g->opens = OPEN_NOTHING;
    /* Having read what the futon is, the same question follows. */
    if (read_out && was == D_X_FUTON && g->phase == PHASE_BEFORE)
      ask_about_the_night(g);
    return;
  }
  case GAME_INVENTORY:
    inventory_action(g, a);
    return;
  case GAME_ENCOUNTER:
    encounter_action(g, a);
    return;
  case GAME_MEND:
    mend_action(g, a);
    return;
  case GAME_PROMPT:
    prompt_action(g, a);
    return;
  case GAME_EXPLORATION:
    break;
  }
  if (a == ACT_CANCEL) {
    g->state = GAME_NOTEBOOK;
    g->scroll = g->note_count > NOTES_PER_PAGE ? g->note_count - NOTES_PER_PAGE : 0;
    emit(g, EV_NOTEBOOK_OPEN, g->note_count, 0);
    return;
  }
  if (a == ACT_INVENTORY) {
    g->state = GAME_INVENTORY;
    g->selection = 0;
    g->message[0] = 0;
    return;
  }
  if (a == ACT_CONFIRM) {
    int n = game_npc_at(g, g->x + g->dx, g->y + g->dy);
    if (n >= 0)
      talk(g, n);
    else
      examine(g);
    return;
  }
  if (a == ACT_NONE)
    return;
  g->message[0] = 0;
  if (a == ACT_UP)
    move(g, 0, -1);
  else if (a == ACT_DOWN)
    move(g, 0, 1);
  else if (a == ACT_LEFT)
    move(g, -1, 0);
  else if (a == ACT_RIGHT)
    move(g, 1, 0);
}
void game_camera(const Game *g, int *x, int *y) {
  *x = g->x - 10;
  *y = g->y - 5;
  int mx = g->maps[g->map].width - 20, my = g->maps[g->map].height - 10;
  if (*x > mx)
    *x = mx;
  if (*y > my)
    *y = my;
  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
}
