#include "game.h"
#include "../../shared/talk.h"

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
  char digits[3], out[4];
  uint8_t n = 0, k = 0;
  /* Nur Schadenszahlen, also hoechstens dreistellig: acht Bit reichen, und die
   * 16-Bit-Division waere auf dem 6502 eine Hilfsroutine (#25). */
  uint8_t u = v < 0 ? 0 : (v > 255 ? 255 : (uint8_t)v);
  do {
    digits[n++] = (char)('0' + u % 10);
    u = (uint8_t)(u / 10);
  } while (u);
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
  if (map == MAP_FOREST)
    for (uint8_t i = 0; i < STAKE_COUNT; i++)
      if ((g->staked & (1u << i)) && stakes[i].x == (uint8_t)x &&
          stakes[i].y == (uint8_t)y)
        return 'p';
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
void game_npc_pos(const Game *g, uint8_t npc, int8_t *x, int8_t *y) {
  *x = npc == NPC_DAIGO ? g->daigo_x : (int8_t)npcs[npc].x;
  *y = npc == NPC_DAIGO ? g->daigo_y : (int8_t)npcs[npc].y;
}
int8_t game_npc_at(const Game *g, int8_t x, int8_t y) {
  for (uint8_t i = 0; i < NPC_COUNT; i++) {
    int8_t nx, ny;
    game_npc_pos(g, i, &nx, &ny);
    if (npcs[i].map == g->map && nx == x && ny == y)
      return (int8_t)i;
  }
  return -1;
}
uint8_t game_mend_pieces(const Game *g, uint8_t *out) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < MEND_PIECES; i++)
    if (mend_display[i] >= g->mend_placed)
      out[n++] = mend_display[i];
  return n;
}
uint8_t game_owned_items(const Game *g, uint8_t *out) {
  uint8_t n = 0;
  for (uint8_t i = ITEM_NONE + 1; i < ITEM_COUNT; i++)
    if (g->bag[i])
      out[n++] = i;
  return n;
}

static void cue(Game *g, SfxId id) {
  if (id > g->sfx)
    g->sfx = (uint8_t)id;
}
/* The C64 keeps no event, only the sound that follows from it (see feedback.h). */
static void emit(Game *g, EventType type, int a, int b) {
  cue(g, sfx_for_event(type, a, b));
}
static void finish(Game *g, uint8_t outcome) {
  if (g->outcome != OUT_NONE)
    return;
  g->outcome = outcome;
  emit(g, EV_OUTCOME, outcome, 0);
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
    cue(g, SFX_WRITE);
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
static void open_dialogue(Game *g, int8_t npc, char examined, uint8_t dialogue,
                          uint8_t opens) {
  g->opens = opens;
  g->npc = npc;
  g->examined = examined;
  g->dialogue = dialogue;
  g->page = 0;
  msg_clear(g);
  g->state = GAME_DIALOGUE;
}
/* Moves the selection highlight up or down within a list of `count` entries. */
static void select_move(Game *g, Action a, uint8_t count) {
  if (a == ACT_UP)
    g->selection = (uint8_t)((g->selection + count - 1) % count);
  else if (a == ACT_DOWN)
    g->selection = (uint8_t)((g->selection + 1) % count);
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
  g->daigo_x = (int8_t)npcs[NPC_DAIGO].x;
  g->daigo_y = (int8_t)npcs[NPC_DAIGO].y;
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

/* Auf einer Karte ankommen -- durchs Tor gegangen oder heimgetragen, das
 * macht fuer die Karte keinen Unterschied: Daigo bleibt in seinem Wald, und
 * der Ort nennt sich neu. */
static void arrive(Game *g, uint8_t map, int8_t x, int8_t y) {
  g->map = map;
  g->x = x;
  g->y = y;
  if (g->daigo_follows) { /* he stays in the forest, at his camp */
    g->daigo_follows = false;
    g->daigo_x = (int8_t)npcs[NPC_DAIGO].x;
    g->daigo_y = (int8_t)npcs[NPC_DAIGO].y;
  }
  g->place = 0; /* the name belongs to the room actually entered */
  enter_place(g);
}

/* --- talking --- */
static void talk(Game *g, int8_t npc) {
  const DialogueRule *r = talk_select(npc, g->obs, g->outcome, g->phase, g->bag);
  if (!r)
    return;
  if (r->gives != ITEM_NONE)
    g->bag[r->gives]++;
  learn(g, r->grants, r->note);
  open_dialogue(g, npc, 0, r->dialogue, r->opens);
}

/* --- examining --- */
static bool stone_at(const Game *g, Coord x, Coord y);
static void reset_stone(Game *g);
static bool plain_tile(const TileDef *t);
static void use_point(Game *g, const ExaminePoint *p, char examined) {
  if (p->kind == POINT_STONE && p->dialogue == D_X_STONE_STUCK)
    reset_stone(g);
  if (p->takes != ITEM_NONE && g->bag[p->takes])
    g->bag[p->takes]--;
  if (p->gives != ITEM_NONE)
    g->bag[p->gives]++;
  learn(g, p->grants, p->note);
  open_dialogue(g, -1, examined, p->dialogue, OPEN_NOTHING);
}
static void examine_nothing(Game *g, int8_t x, int8_t y) {
  const TileDef *tile = tile_def(game_tile(g, g->map, x, y));
  msg_clear(g);
  msg_add(g, tile ? tile->name : "Dort");
  msg_add(g, ": nichts Besonderes.");
}
static bool stake_here(Game *g);
#include "../../shared/examine.h"
static void move(Game *g, int8_t dx, int8_t dy);
/* Ein Schritt, wenn die Taste eine Richtung war. */
static bool walk(Game *g, Action a) {
  if (a == ACT_UP)
    move(g, 0, -1);
  else if (a == ACT_DOWN)
    move(g, 0, 1);
  else if (a == ACT_LEFT)
    move(g, -1, 0);
  else if (a == ACT_RIGHT)
    move(g, 1, 0);
  else
    return false;
  return true;
}

/* --- the bag --- */
static bool heal(Game *g) {
  if (!g->bag[ITEM_HERB] || g->hp >= PLAYER_HP)
    return false;
  g->bag[ITEM_HERB]--;
  g->hp += items[ITEM_HERB].heal;
  if (g->hp > PLAYER_HP)
    g->hp = PLAYER_HP;
  return true;
}
static void inventory_action(Game *g, Action a) {
  uint8_t owned[ITEM_COUNT];
  uint8_t count = game_owned_items(g, owned);
  if (a == ACT_CANCEL || a == ACT_INVENTORY) {
    g->state = GAME_EXPLORATION;
    return;
  }
  /* Die Tasche haelt niemanden fest: eine Richtung, in der es hier nichts zu
   * waehlen gibt, schliesst sie und geht den Schritt (#19). */
  bool selecting = count > 1 && (a == ACT_UP || a == ACT_DOWN);
  if (!selecting && a != ACT_CONFIRM) {
    uint8_t was = g->state;
    g->state = GAME_EXPLORATION;
    msg_clear(g);
    if (!walk(g, a))
      g->state = was; /* keine Richtung: die Tasche bleibt offen */
    return;
  }
  if (count == 0)
    return;
  select_move(g, a, count);
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  uint8_t item = owned[g->selection];
  const ExaminePoint *p = point_for_item(g, item);
  if (p) {
    emit(g, EV_ITEM_USE, item, p->dialogue);
    use_point(g, p, p->symbol);
    return;
  }
  bool healed = item == ITEM_HERB && heal(g);
  msg_clear(g);
  msg_add(g, healed              ? "Das Kraut lindert deine Wunden."
             : item == ITEM_HERB ? "Du bist unverletzt."
                                 : "Damit kannst du hier nichts tun.");
}
static void notebook_action(Game *g, Action a) {
  uint8_t last =
      g->note_count > NOTES_PER_PAGE ? (uint8_t)(g->note_count - NOTES_PER_PAGE) : 0;
  if (a == ACT_CANCEL || a == ACT_CONFIRM)
    g->state = GAME_EXPLORATION;
  else if (a == ACT_UP && g->scroll > 0)
    g->scroll--;
  else if (a == ACT_DOWN && g->scroll < last)
    g->scroll++;
}

/* Kintsugi: each piece is set into the gap it belongs to. A piece that does not
 * fit costs nothing; the seams stay visible. */
/* One line of a dialogue as the message, replacing what stood there. */
static void show(Game *g, uint8_t line) {
  msg_clear(g);
  msg_add(g, dialogues[line].pages[0]);
}
static void take_item(Game *g, uint8_t item) {
  if (g->bag[item])
    g->bag[item]--;
}
#include "../../shared/mend.h"

/* Driving in a stake: only where the tracks run, and only with Daigo there. */
#include "../../shared/stake.h"

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
static void carried_home(Game *g) {
  for (uint8_t i = 0; i < TRANSITION_COUNT; i++)
    if (transitions[i].to_map == MAP_VILLAGE) {
      arrive(g, MAP_VILLAGE, (int8_t)transitions[i].to_x, (int8_t)transitions[i].to_y);
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
      msg_add(g, FIGHT_NO_HERB_TEXT);
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
    msg_add(g, FIGHT_HIT_TEXT);
    msg_num(g, hit);
    msg_add(g, FIGHT_WON_TEXT);
    learn(g, 0, N_FOUGHT);
    g->outcome = OUT_FIGHT;
    emit(g, EV_OUTCOME, OUT_FIGHT, 0);
    open_scene(g, "Am Rand des Hains", D_ENC_VICTORY);
    return;
  }
  int16_t taken =
      damage(KAMI_ATTACK + (g->mood == MOOD_ANGRY ? 1 : 0), PLAYER_DEFENSE, roll(g));
  g->hp -= taken;
  if (g->hp <= 0) {
    g->hp = PLAYER_HP;
    g->kami_hp = KAMI_HP; /* the spirit recovers as well */
    g->fighting = 0;
    carried_home(g);
    open_scene(g, "Kiriyama, spaeter", D_ENC_DEFEAT);
    return;
  }
  msg_clear(g);
  if (herb)
    msg_add(g, FIGHT_HERB_TEXT);
  else {
    msg_add(g, FIGHT_HIT_TEXT);
    msg_num(g, hit);
    msg_add(g, FIGHT_THEN_TEXT);
  }
  msg_num(g, taken);
  msg_add(g, FIGHT_END_TEXT);
}
static bool carries(const Game *g, uint8_t item) { return g->bag[item] != 0; }
/* The panel reads the line from the dialogue table. */
static void encounter_say(Game *g, Count line) {
  g->dialogue = line;
  g->page = 0;
}
#include "../../shared/encounter.h"

/* --- walking and pushing --- */
/* The follower steps aside (they swap), everyone else blocks. */
static bool blocking_npc(const Game *g, int8_t x, int8_t y) {
  int8_t npc = game_npc_at(g, x, y);
  return npc >= 0 && !(npc == NPC_DAIGO && g->daigo_follows);
}
static bool can_enter(const Game *g, int8_t x, int8_t y) {
  return !blocking_npc(g, x, y) && game_passable(g, g->map, x, y);
}
/* Pushing the stone one tile. Nothing here knows why it matters: the
 * inscription and the empty hollow say that, and the player draws the line. */
/* An ordinary tile: walkable, neither guarded nor a way to another map. */
static bool plain_tile(const TileDef *t) {
  return t && (t->flags & TF_PASSABLE) && !(t->flags & (TF_GUARDED | TF_TRANSITION));
}
#include "../../shared/stone.h"
/* Ein versperrter Schritt schweigt beim ersten Mal -- wer sieht, wogegen er
 * laeuft, braucht keinen Text. Erst der zweite Versuch in dieselbe Richtung
 * bekommt eine Antwort (#20). */
static void blocked(Game *g, int8_t dx, int8_t dy, const char *what,
                    const char *tile_name) {
  if (g->blocked_dx != dx || g->blocked_dy != dy) {
    g->blocked_dx = dx;
    g->blocked_dy = dy;
    return;
  }
  msg_clear(g);
  if (what)
    msg_add(g, what);
  if (tile_name) {
    msg_add(g, tile_name);
    msg_add(g, " versperrt den Weg.");
  }
}
static void move(Game *g, int8_t dx, int8_t dy) {
  g->dx = dx;
  g->dy = dy;
  int8_t tx = (int8_t)(g->x + dx), ty = (int8_t)(g->y + dy);
  if (stone_at(g, tx, ty)) {
    if (push_stone(g, dx, dy)) {
      g->x += dx;
      g->y += dy;
      g->blocked_dx = 0;
      g->blocked_dy = 0;
    } else /* er laesst sich bewegen -- nur weiss man noch nicht, warum */
      blocked(g, dx, dy, "Der Grenzstein ruehrt sich nicht.", 0);
    return;
  }
  if (!can_enter(g, tx, ty)) {
    if (game_npc_at(g, tx, ty) >= 0)
      blocked(g, dx, dy, "Da steht jemand im Weg.", 0);
    else {
      const TileDef *t = tile_def(game_tile(g, g->map, tx, ty));
      blocked(g, dx, dy, 0, t ? t->name : "Etwas");
    }
    return;
  }
  g->blocked_dx = 0;
  g->blocked_dy = 0;
  const TileDef *tile = tile_def(game_tile(g, g->map, g->x + dx, g->y + dy));
  if ((tile->flags & TF_GUARDED) && g->outcome == OUT_NONE) {
    begin_encounter(g); /* a settled grove lets you pass */
    return;
  }
  int8_t from_x = g->x, from_y = g->y;
  g->x += dx;
  g->y += dy;
  cue(g, (g->x + g->y) & 1 ? SFX_STEP_A : SFX_STEP_B); /* every step flips it */
  if (g->daigo_follows) {                              /* he walks in your footsteps */
    g->daigo_x = from_x;
    g->daigo_y = from_y;
  }
  if (!place_at(g, g->map, g->x, g->y))
    g->place = 0; /* outside again: the next room may repeat its name */
  else
    enter_place(g);
  if (!(tile->flags & TF_TRANSITION))
    return;
  for (uint8_t i = 0; i < TRANSITION_COUNT; i++) {
    const Transition *t = &transitions[i];
    if (g->map == t->map && (uint8_t)g->x == t->x && (uint8_t)g->y == t->y) {
      bool from_forest = g->map == MAP_FOREST;
      arrive(g, t->to_map, (int8_t)t->to_x, (int8_t)t->to_y);
      /* The lacquer needs rest: it has dried by the time you are back. */
      if (from_forest && g->map == MAP_VILLAGE && game_knows(g, OBS_BOWL_DRYING))
        learn(g, OBS(OBS_BOWL_READY), NOTE_NONE);
      return;
    }
  }
}

#include "../../shared/night.h"

void game_action(Game *g, Action a) {
  g->sfx = SFX_NONE;
  if (g->state != GAME_EXPLORATION && (a == ACT_CONFIRM || a == ACT_CANCEL))
    cue(g, SFX_CLICK);
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
  case GAME_DIALOGUE: {
    if (a != ACT_CANCEL &&
        !(a == ACT_CONFIRM && ++g->page >= dialogues[g->dialogue].count))
      return;
    bool read_out = a == ACT_CONFIRM;
    Count was = g->dialogue;
    g->state = GAME_EXPLORATION;
    msg_clear(g); /* the scene's echo of the last round ends with it */
    /* Escape closes and nothing more; reading to the end can lead on. */
    if (a == ACT_CONFIRM && g->opens == OPEN_MEND) {
      g->state = GAME_MEND;
      g->selection = 0;
    } else if (a == ACT_CONFIRM && g->opens == OPEN_FOLLOW)
      g->daigo_follows = true;
    else if (a == ACT_CONFIRM && g->opens == OPEN_END && !g->ended) {
      g->ended = true; /* einmal, danach laeuft die Welt weiter */
      g->state = GAME_END;
    }
    ask_after_dialogue(g, read_out,
                       was); /* the night, or the futon that explained itself */
    g->opens = OPEN_NOTHING;
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
  case GAME_END:
    if (a == ACT_CONFIRM || a == ACT_CANCEL)
      g->state = GAME_EXPLORATION;
    return;
  case GAME_EXPLORATION:
    break;
  }
  if (a == ACT_CANCEL) {
    g->state = GAME_NOTEBOOK;
    g->scroll =
        g->note_count > NOTES_PER_PAGE ? (uint8_t)(g->note_count - NOTES_PER_PAGE) : 0;
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
  walk(g, a);
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
