#include "game.h"

/* --- small helpers instead of stdio: the C64 build has no printf --- */
static void msg_clear(Game *g) { g->message[0] = 0; }
static void msg_add(Game *g, const char *s) {
  uint8_t n = 0;
  while (g->message[n])
    n++;
  while (*s && n < MESSAGE_LIMIT - 1)
    g->message[n++] = *s++;
  g->message[n] = 0;
}
static void msg_num(Game *g, int16_t v) {
  char digits[6], out[7];
  uint8_t n = 0, k = 0;
  if (v < 0)
    v = 0;
  do {
    digits[n++] = (char)('0' + v % 10);
    v /= 10;
  } while (v);
  while (n)
    out[k++] = digits[--n];
  out[k] = 0;
  msg_add(g, out);
}

static bool matches(const Game *g, Obs needs, Obs forbids) {
  return (g->obs & needs) == needs && !(g->obs & forbids);
}
static bool fits_now(const Game *g, Obs needs, uint8_t outcome) {
  return (g->obs & needs) == needs && (outcome == OUT_ANY || outcome == g->outcome);
}
bool game_knows(const Game *g, ObsId o) { return (g->obs & OBS(o)) != 0; }

char game_tile(const Game *g, uint8_t map, int8_t x, int8_t y) {
  if (map == MAP_FOREST && x == g->stone_x && y == g->stone_y)
    return 'G'; /* the stone the loggers dragged out of its hollow */
  for (uint8_t i = 0; i < tile_override_count; i++) {
    const TileOverride *o = &tile_overrides[i];
    if (o->map == map && o->x == (uint8_t)x && o->y == (uint8_t)y &&
        fits_now(g, o->needs, o->outcome))
      return o->symbol;
  }
  return map_at(map, x, y);
}
bool game_shows(const Game *g, const TileOverride *o) {
  return fits_now(g, o->needs, o->outcome);
}
bool game_passable(const Game *g, uint8_t map, int8_t x, int8_t y) {
  const TileDef *t = tile_def(game_tile(g, map, x, y));
  return t && (t->flags & TF_PASSABLE);
}
int8_t game_npc_at(const Game *g, int8_t x, int8_t y) {
  for (uint8_t i = 0; i < NPC_COUNT; i++)
    if (npcs[i].map == g->map && npcs[i].x == (uint8_t)x && npcs[i].y == (uint8_t)y)
      return (int8_t)i;
  return -1;
}
uint8_t game_owned_items(const Game *g, uint8_t *out) {
  uint8_t n = 0;
  for (uint8_t i = ITEM_NONE + 1; i < ITEM_COUNT; i++)
    if (g->bag[i])
      out[n++] = i;
  return n;
}

static void learn(Game *g, Obs grants, uint8_t note) {
  g->obs |= grants;
  if (note == NOTE_NONE)
    return;
  for (uint8_t i = 0; i < g->note_count; i++)
    if (g->notes[i] == note)
      return;
  if (g->note_count < NOTE_LIMIT) {
    g->notes[g->note_count++] = note;
    g->note_ticks = NOTE_TICKS; /* only a new entry announces itself */
  }
}
/* Keeps g->message: a scene page left empty shows it (the closing round). */
static void open_scene(Game *g, const char *title, uint8_t dialogue) {
  g->npc = SPEAKER_SCENE;
  g->scene = title;
  g->examined = 0;
  g->dialogue = dialogue;
  g->page = 0;
  g->state = GAME_DIALOGUE;
}
static void open_dialogue(Game *g, int8_t npc, char examined, uint8_t dialogue) {
  g->npc = npc;
  g->examined = examined;
  g->dialogue = dialogue;
  g->page = 0;
  msg_clear(g);
  g->state = GAME_DIALOGUE;
}

void game_init(Game *g) {
  world_init();
  for (uint16_t i = 0; i < sizeof *g; i++)
    ((char *)g)[i] = 0;
  g->map = MAP_FOREST;
  g->x = 24;
  g->y = 19;
  g->dy = -1;
  g->hp = PLAYER_HP;
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  g->kami_hp = KAMI_HP;
  g->random = 42;
  g->npc = SPEAKER_SCENE;
  g->scene = "Unterwegs";
  g->dialogue = D_SCENE_ARRIVAL;
  g->state = GAME_TITLE; /* the arrival scene waits behind the title page */
}

/* --- the place a room names itself by --- */
static const Place *place_at(const Game *g, uint8_t map, int8_t x, int8_t y) {
  for (uint8_t i = 0; i < place_count; i++) {
    const Place *p = &places[i];
    if (p->map != map || !matches(g, p->needs, 0))
      continue;
    if ((uint8_t)x >= p->x && (uint8_t)x < p->x + p->width && (uint8_t)y >= p->y &&
        (uint8_t)y < p->y + p->height)
      return p;
  }
  return 0;
}
static void enter_place(Game *g) {
  const Place *p = place_at(g, g->map, g->x, g->y);
  if (!p || g->place == p->name)
    return;
  g->place = p->name;
  g->place_ticks = PLACE_TICKS;
}

/* --- talking --- */
static void talk(Game *g, int8_t npc) {
  for (uint8_t i = 0; i < dialogue_rule_count; i++) {
    const DialogueRule *r = &dialogue_rules[i];
    if (r->npc != (uint8_t)npc || !matches(g, r->needs, r->forbids))
      continue;
    if (!fits_now(g, 0, r->outcome))
      continue;
    /* Do not hand over something the player still carries: the next rule speaks. */
    if (r->gives != ITEM_NONE && g->bag[r->gives])
      continue;
    if (r->gives != ITEM_NONE)
      g->bag[r->gives]++;
    learn(g, r->grants, r->note);
    open_dialogue(g, npc, 0, r->dialogue);
    return;
  }
}

/* --- examining --- */
static bool stone_at(const Game *g, int8_t x, int8_t y) {
  return g->map == MAP_FOREST && x == g->stone_x && y == g->stone_y;
}
static const ExaminePoint *point_at(const Game *g, int8_t x, int8_t y) {
  char symbol = game_tile(g, g->map, x, y);
  for (uint8_t i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_ITEM || !matches(g, p->needs, 0) || p->map != g->map)
      continue;
    if (p->kind == POINT_STONE) {
      if (stone_at(g, x, y))
        return p;
      continue;
    }
    if (p->kind == POINT_AT ? p->x == (uint8_t)x && p->y == (uint8_t)y
                            : p->symbol == symbol)
      return p;
  }
  return 0;
}
static const ExaminePoint *point_for_item(const Game *g, uint8_t item) {
  char facing = game_tile(g, g->map, g->x + g->dx, g->y + g->dy);
  for (uint8_t i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind != POINT_ITEM || p->item != item || !matches(g, p->needs, 0))
      continue;
    if (!p->symbol || (p->map == g->map && p->symbol == facing))
      return p;
  }
  return 0;
}
/* Rolling the stuck stone back to where the drag marks start. */
static void reset_stone(Game *g) {
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  g->obs &= ~OBS(OBS_STONE_MOVED);
}
static void use_point(Game *g, const ExaminePoint *p, char examined) {
  if (p->kind == POINT_STONE && p->dialogue == D_X_STONE_STUCK)
    reset_stone(g);
  if (p->takes != ITEM_NONE && g->bag[p->takes])
    g->bag[p->takes]--;
  if (p->gives != ITEM_NONE)
    g->bag[p->gives]++;
  learn(g, p->grants, p->note);
  open_dialogue(g, -1, examined, p->dialogue);
}
static void examine_nothing(Game *g, int8_t x, int8_t y) {
  const TileDef *tile = tile_def(game_tile(g, g->map, x, y));
  msg_clear(g);
  msg_add(g, tile ? tile->name : "Dort");
  msg_add(g, ": nichts Besonderes.");
}
/* Facing tile first, then the tile underfoot (passable points cannot be faced). */
static void examine(Game *g) {
  int8_t fx = g->x + g->dx, fy = g->y + g->dy;
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

/* --- the bag --- */
static bool heal(Game *g) {
  if (!g->bag[ITEM_HERB] || g->hp >= PLAYER_HP)
    return false;
  g->bag[ITEM_HERB]--;
  g->hp += items[ITEM_HERB].heal;
  if (g->hp > PLAYER_HP)
    g->hp = PLAYER_HP;
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
  return true;
}
static void inventory_action(Game *g, Action a) {
  uint8_t owned[ITEM_COUNT];
  uint8_t count = game_owned_items(g, owned);
  if (a == ACT_CANCEL || a == ACT_INVENTORY) {
    g->state = GAME_EXPLORATION;
    return;
  }
  if (count == 0)
    return;
  if (a == ACT_UP)
    g->selection = (uint8_t)((g->selection + count - 1) % count);
  if (a == ACT_DOWN)
    g->selection = (uint8_t)((g->selection + 1) % count);
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  uint8_t item = owned[g->selection];
  const ExaminePoint *p = point_for_item(g, item);
  if (p) {
    use_point(g, p, p->symbol);
    return;
  }
  bool healed = item == ITEM_HERB && heal(g);
  msg_clear(g);
  msg_add(g, healed              ? "Das Kraut lindert deine Wunden."
          : item == ITEM_HERB    ? "Du bist unverletzt."
                                 : "Damit kannst du hier nichts tun.");
}
static void notebook_action(Game *g, Action a) {
  uint8_t last = g->note_count > NOTES_PER_PAGE ? (uint8_t)(g->note_count - NOTES_PER_PAGE) : 0;
  if (a == ACT_CANCEL || a == ACT_CONFIRM)
    g->state = GAME_EXPLORATION;
  else if (a == ACT_UP && g->scroll > 0)
    g->scroll--;
  else if (a == ACT_DOWN && g->scroll < last)
    g->scroll++;
}

/* --- the encounter --- */
static const EncounterOffer *offer_at_hand(const Game *g) {
  for (uint8_t i = 0; i < encounter_offer_count; i++)
    if (g->bag[encounter_offers[i].item])
      return &encounter_offers[i];
  return 0;
}
uint8_t game_encounter_options(const Game *g, uint8_t *out) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < encounter_option_count; i++) {
    const EncounterOption *o = &encounter_options[i];
    if (o->when != OPT_BOTH && (o->when == OPT_FIGHT) != (g->fighting != 0))
      continue;
    if (o->action == ENC_OFFER && !offer_at_hand(g))
      continue;
    if (o->action == ENC_HEAL && !g->bag[ITEM_HERB])
      continue;
    out[n++] = i;
  }
  return n;
}
static int16_t damage(int16_t attack, int16_t defense, int16_t modifier) {
  int16_t d = attack - defense + modifier;
  return d < 1 ? 1 : d;
}
static int16_t roll(Game *g) {
  g->random = g->random * 1664525u + 1013904223u;
  return (int16_t)((g->random >> 16) % 3) - 1;
}
/* Guarded ground pushes the player one step away, onto free, safe ground. */
static bool can_enter(const Game *g, int8_t x, int8_t y);
static void step_back(Game *g) {
  int8_t bx = g->x - g->dx, by = g->y - g->dy;
  const TileDef *back = tile_def(game_tile(g, g->map, bx, by));
  if (can_enter(g, bx, by) && back && !(back->flags & (TF_GUARDED | TF_TRANSITION))) {
    g->x = bx;
    g->y = by;
  }
}
static void carried_home(Game *g) {
  for (uint8_t i = 0; i < TRANSITION_COUNT; i++)
    if (transitions[i].to_map == MAP_VILLAGE) {
      g->map = MAP_VILLAGE;
      g->x = (int8_t)transitions[i].to_x;
      g->y = (int8_t)transitions[i].to_y;
      break;
    }
  g->dx = 0;
  g->dy = 1;
}
/* One exchange of blows. The fight continues until someone falls or the player
 * steps back; the spirit keeps its wounds until it wins. */
static void fight_round(Game *g, bool herb) {
  if (!g->fighting) {
    g->fighting = 1;
    if (g->kami_hp <= 0)
      g->kami_hp = KAMI_HP;
  }
  int16_t hit = 0;
  if (herb) {
    if (!heal(g)) {
      msg_clear(g);
      msg_add(g, "Kein Heilkraut benutzt. Waehle neu.");
      return;
    }
  } else {
    hit = damage(PLAYER_ATTACK, KAMI_DEFENSE, roll(g));
    g->kami_hp -= hit;
  }
  if (g->kami_hp <= 0) {
    g->kami_hp = 0;
    g->fighting = 0;
    msg_clear(g);
    msg_add(g, "Dein Hieb verursacht ");
    msg_num(g, hit);
    msg_add(g, " Schaden.\nDer Kami sinkt in sich zusammen.");
    learn(g, 0, N_FOUGHT);
    g->outcome = OUT_FIGHT;
    open_scene(g, "Am Rand des Hains", D_ENC_VICTORY);
    return;
  }
  int16_t taken = damage(KAMI_ATTACK + (g->mood == MOOD_ANGRY ? 1 : 0), PLAYER_DEFENSE,
                         roll(g));
  g->hp -= taken;
  if (g->hp <= 0) {
    g->hp = PLAYER_HP;
  g->stone_x = STONE_START_X;
  g->stone_y = STONE_START_Y;
    g->kami_hp = KAMI_HP; /* the spirit recovers as well */
    g->fighting = 0;
    carried_home(g);
    open_scene(g, "Kiriyama, spaeter", D_ENC_DEFEAT);
    return;
  }
  msg_clear(g);
  if (herb)
    msg_add(g, "Das Kraut lindert deine Wunden.\nDer Kami verursacht ");
  else {
    msg_add(g, "Dein Hieb verursacht ");
    msg_num(g, hit);
    msg_add(g, " Schaden.\nDer Kami verursacht ");
  }
  msg_num(g, taken);
  msg_add(g, " Schaden.");
}
/* The spirit rises from the grove; its mood is remembered between encounters. */
static void begin_encounter(Game *g) {
  g->state = GAME_ENCOUNTER;
  g->selection = 0;
  learn(g, OBS(OBS_KAMI_SEEN), N_KAMI);
  g->dialogue = D_ENC_APPEAR;
  g->page = 0;
  msg_clear(g);
}
static void encounter_action(Game *g, Action a) {
  uint8_t options[ENCOUNTER_OPTION_LIMIT];
  uint8_t count = game_encounter_options(g, options);
  if (count == 0)
    return;
  if (a == ACT_UP)
    g->selection = (uint8_t)((g->selection + count - 1) % count);
  if (a == ACT_DOWN)
    g->selection = (uint8_t)((g->selection + 1) % count);
  if (a == ACT_CANCEL) /* Escape highlights retreating, it does not do it */
    for (uint8_t i = 0; i < count; i++)
      if (encounter_options[options[i]].action == ENC_RETREAT)
        g->selection = i;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  uint8_t action = encounter_options[options[g->selection]].action;
  const EncounterOffer *offer = action == ENC_OFFER ? offer_at_hand(g) : 0;
  uint8_t before = g->mood;
  g->mood = offer ? offer->result : encounter_transitions[action][before];
  if (offer) {
    learn(g, offer->grants, offer->note);
    g->dialogue = offer->dialogue;
  } else
    g->dialogue = encounter_lines[action][before];
  g->page = 0;
  if (action == ENC_ATTACK || action == ENC_HEAL) {
    fight_round(g, action == ENC_HEAL);
    g->selection = 0;
    return;
  }
  if (action == ENC_RETREAT) {
    step_back(g);
    g->fighting = 0;
    g->state = GAME_EXPLORATION;
  }
}

/* --- walking and pushing --- */
static bool can_enter(const Game *g, int8_t x, int8_t y) {
  return game_npc_at(g, x, y) < 0 && game_passable(g, g->map, x, y);
}
bool game_can_push(const Game *g) {
  return matches(g, OBS(OBS_STONE_DRAGGED) | OBS(OBS_STONE_HOLLOW), 0) &&
         g->outcome == OUT_NONE;
}
/* Pushing the stone one tile. Nothing here knows why it matters: the
 * inscription and the empty hollow say that, and the player draws the line. */
static bool push_stone(Game *g, int8_t dx, int8_t dy) {
  int8_t tx = (int8_t)(g->stone_x + dx), ty = (int8_t)(g->stone_y + dy);
  const TileDef *target = tile_def(game_tile(g, MAP_FOREST, tx, ty));
  if (!game_can_push(g) || !target || !(target->flags & TF_PASSABLE) ||
      (target->flags & (TF_GUARDED | TF_TRANSITION)) || game_npc_at(g, tx, ty) >= 0)
    return false;
  g->stone_x = tx;
  g->stone_y = ty;
  bool settled = tx == STONE_HOLLOW_X && ty == STONE_HOLLOW_Y;
  if (settled)
    g->obs &= ~OBS(OBS_STONE_MOVED);
  else
    g->obs |= OBS(OBS_STONE_MOVED);
  if (settled) {
    g->mood = MOOD_CALM;
    learn(g, 0, N_BOUNDARY);
    g->outcome = OUT_BOUNDARY;
    open_scene(g, "Die alte Grenze", D_SCENE_BOUNDARY);
  }
  return true;
}
static void move(Game *g, int8_t dx, int8_t dy) {
  g->dx = dx;
  g->dy = dy;
  if (stone_at(g, (int8_t)(g->x + dx), (int8_t)(g->y + dy))) {
    if (push_stone(g, dx, dy)) {
      g->x += dx;
      g->y += dy;
    }
    return;
  }
  if (!can_enter(g, g->x + dx, g->y + dy))
    return;
  const TileDef *tile = tile_def(game_tile(g, g->map, g->x + dx, g->y + dy));
  if ((tile->flags & TF_GUARDED) && g->outcome == OUT_NONE) {
    begin_encounter(g); /* a settled grove lets you pass */
    return;
  }
  g->x += dx;
  g->y += dy;
  if (!place_at(g, g->map, g->x, g->y))
    g->place = 0; /* outside again: the next room may repeat its name */
  else
    enter_place(g);
  if (!(tile->flags & TF_TRANSITION))
    return;
  for (uint8_t i = 0; i < TRANSITION_COUNT; i++) {
    const Transition *t = &transitions[i];
    if (g->map == t->map && (uint8_t)g->x == t->x && (uint8_t)g->y == t->y) {
      g->map = t->to_map;
      g->x = (int8_t)t->to_x;
      g->y = (int8_t)t->to_y;
      g->place = 0; /* the name belongs to the room actually entered */
      enter_place(g);
      return;
    }
  }
}

void game_action(Game *g, Action a) {
  if (g->place_ticks)
    g->place_ticks--;
  if (g->note_ticks)
    g->note_ticks--;
  switch (g->state) {
  case GAME_TITLE:
    if (a == ACT_CONFIRM)
      g->state = GAME_DIALOGUE;
    return;
  case GAME_NOTEBOOK:
    notebook_action(g, a);
    return;
  case GAME_DIALOGUE:
    if (a != ACT_CANCEL && !(a == ACT_CONFIRM && ++g->page >= dialogues[g->dialogue].count))
      return;
    g->state = GAME_EXPLORATION;
    msg_clear(g); /* the scene's echo of the last round ends with it */
    return;
  case GAME_INVENTORY:
    inventory_action(g, a);
    return;
  case GAME_ENCOUNTER:
    encounter_action(g, a);
    return;
  case GAME_EXPLORATION:
    break;
  }
  if (a == ACT_CANCEL) {
    g->state = GAME_NOTEBOOK;
    g->scroll = g->note_count > NOTES_PER_PAGE ? (uint8_t)(g->note_count - NOTES_PER_PAGE) : 0;
    return;
  }
  if (a == ACT_INVENTORY) {
    g->state = GAME_INVENTORY;
    g->selection = 0;
    msg_clear(g);
    return;
  }
  if (a == ACT_CONFIRM) {
    int8_t n = game_npc_at(g, g->x + g->dx, g->y + g->dy);
    if (n >= 0)
      talk(g, n);
    else
      examine(g);
    return;
  }
  if (a == ACT_NONE)
    return;
  msg_clear(g);
  if (a == ACT_UP)
    move(g, 0, -1);
  else if (a == ACT_DOWN)
    move(g, 0, 1);
  else if (a == ACT_LEFT)
    move(g, -1, 0);
  else if (a == ACT_RIGHT)
    move(g, 1, 0);
}

void game_camera(const Game *g, uint8_t view_w, uint8_t view_h, int8_t *x, int8_t *y) {
  int16_t cx = g->x - view_w / 2, cy = g->y - view_h / 2;
  int16_t mx = map_width(g->map) - view_w, my = map_height(g->map) - view_h;
  if (cx > mx)
    cx = mx;
  if (cy > my)
    cy = my;
  if (cx < 0)
    cx = 0;
  if (cy < 0)
    cy = 0;
  *x = (int8_t)cx;
  *y = (int8_t)cy;
}
