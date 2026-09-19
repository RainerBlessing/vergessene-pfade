#ifndef GAME_H
#define GAME_H
#include "combat.h"
#include "content.h"
#include "inventory.h"
#include "world.h"
typedef enum {
  ACT_NONE,
  ACT_UP,
  ACT_DOWN,
  ACT_LEFT,
  ACT_RIGHT,
  ACT_CONFIRM,
  ACT_CANCEL,
  ACT_INVENTORY,
  ACT_DEBUG,
  ACT_COLLISION
} Action;
typedef enum {
  GAME_EXPLORATION,
  GAME_DIALOGUE,
  GAME_INVENTORY,
  GAME_NOTEBOOK,
  GAME_ENCOUNTER,
  GAME_MEND,
  GAME_PROMPT
} GameState;

/* Session-log events; the frontend drains them with game_take_events. */
typedef enum {
  EV_OBSERVE,          /* a = ObsId */
  EV_EXAMINE,          /* a = tile symbol or 0 for items, b = DialogueId */
  EV_EXAMINE_NOTHING,  /* a = tile symbol, b = suppressed repeats of the previous one */
  EV_NPC_TALK,         /* a = NpcId, b = DialogueId */
  EV_NOTEBOOK_OPEN,    /* a = number of entries */
  EV_ITEM_USE,         /* a = ItemId, b = DialogueId, or D_NONE when nothing happened */
  EV_ENCOUNTER,        /* a = Mood at the start */
  EV_ENCOUNTER_ACTION, /* a = EncounterAction, b = Mood afterwards */
  EV_OUTCOME,          /* a = Outcome */
  EV_STONE_PUSH,       /* a,b = the stone's position after moving it */
  EV_MEND,             /* a = pieces in place, b = 1 when the piece fitted */
  EV_STAKE,            /* a = stakes set */
  EV_PHASE,            /* a = Phase */
  EV_STEP              /* a = tile stepped onto */
} EventType;
typedef struct {
  EventType type;
  int map, x, y, a, b;
} GameEvent;
#define EVENT_LIMIT 32
#define NOTE_LIMIT 32
#define NOTES_PER_PAGE 4

typedef struct {
  bool valid;
  int map, x, y, dx, dy, repeat;
} NothingTarget;
typedef struct {
  Map maps[MAP_COUNT];
  int map, x, y, dx, dy;
  GameState state;
  /* Dialogue speaker: an NPC, SPEAKER_SCENE, or -1 with the examined tile symbol
   * (0 for items). */
  int npc, page, selection, dialogue, scroll;
  char examined;
  const char *scene; /* panel title while npc == SPEAKER_SCENE */
  const char *place; /* the room the player just stepped into */
  int place_ticks;   /* how much longer its name is shown */
  Obs obs;
  NoteId notes[NOTE_LIMIT];
  int note_count;
  Mood mood;
  int stone_x, stone_y; /* the boundary stone the loggers moved */
  int mend_placed;      /* pieces of the bowl already set */
  int daigo_x, daigo_y; /* the foreman walks along while staking the boundary */
  bool daigo_follows;
  uint8_t staked;      /* bit per stake already driven in */
  DialogueOpens opens; /* what the open conversation leads to */
  Combat combat;
  bool fighting;
  Outcome outcome;
  Phase phase;
  Player player;
  char message[128];
  bool debug, collision;
  GameEvent events[EVENT_LIMIT];
  int event_count, events_dropped;
  NothingTarget last_nothing;
} Game;
bool game_init(Game *g, const char *assets);
void game_action(Game *g, Action a);
int game_npc_at(const Game *g, int x, int y);
void game_npc_pos(const Game *g, int npc, int *x, int *y);
void game_camera(const Game *g, int *x, int *y);
/* Map tile after applying overrides for what the player has observed. */
char game_tile(const Game *g, int map, int x, int y);
bool game_passable(const Game *g, int map, int x, int y);
bool game_knows(const Game *g, ObsId o);
/* May the player push the boundary stone? Observations unlock it, not the plot. */
bool game_can_push(const Game *g);
/* Items the player owns, in display order; returns the count. */
int game_owned_items(const Game *g, ItemId *out);
int game_take_events(Game *g, GameEvent *out, int max);
/* Pieces still lying beside the bowl, in display order; returns the count. */
int game_mend_pieces(const Game *g, int *out);
/* Indices into encounter_options offered now, in display order; returns the count.
 * `out` must hold ENCOUNTER_OPTION_LIMIT entries. */
int game_encounter_options(const Game *g, int *out);
/* What "offering" would hand over right now, or ITEM_NONE. */
ItemId game_encounter_offer(const Game *g);
#endif
