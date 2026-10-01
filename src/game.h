#ifndef GAME_H
#define GAME_H
#include "combat.h"
#include "content.h"
#include "inventory.h"
#include "world.h"
#include "../shared/feedback.h"
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
  GAME_TITLE,
  GAME_EXPLORATION,
  GAME_DIALOGUE,
  GAME_INVENTORY,
  GAME_NOTEBOOK,
  GAME_ENCOUNTER,
  GAME_MEND,
  GAME_PROMPT
} GameState;

typedef struct {
  EventType type;
  int map, x, y, a, b;
} GameEvent;
#define EVENT_LIMIT 32
/* Platz fuer jede Notiz: das Buch verliert keine, wie gruendlich man auch sucht. */
#define NOTE_LIMIT (NOTE_COUNT - 1)
#define NOTES_PER_PAGE 4

typedef struct {
  bool valid;
  int map, x, y, dx, dy, repeat;
} NothingTarget;
#include "../shared/state.h"
typedef int Coord;
typedef int Count;
#define MESSAGE_LIMIT 128

typedef struct {
  Map maps[MAP_COUNT];
  int map;
  GameState state;
  GAME_CORE_FIELDS
  unsigned steps; /* counts moves; the frontend turns changes into footfalls */
  Mood mood;
  DialogueOpens opens; /* what the open conversation leads to */
  Combat combat;
  bool fighting;
  Outcome outcome;
  Phase phase;
  Player player;
  bool debug, collision;
  GameEvent events[EVENT_LIMIT];
  int event_count, events_dropped;
  NothingTarget last_nothing;
} Game;
bool game_init(Game *g, const char *assets);
void game_action(Game *g, Action a);
int game_npc_at(const Game *g, int x, int y);
void game_npc_pos(const Game *g, int npc, int *x, int *y);
void game_camera(const Game *g, int view_w, int view_h, int *x, int *y);
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
