#ifndef GAME_H
#define GAME_H
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
typedef enum { GAME_EXPLORATION, GAME_DIALOGUE, GAME_INVENTORY, GAME_NOTEBOOK } GameState;

/* Session-log events; the frontend drains them with game_take_events. */
typedef enum {
  EV_OBSERVE,         /* a = ObsId */
  EV_EXAMINE,         /* a = tile symbol or 0 for items, b = DialogueId */
  EV_EXAMINE_NOTHING, /* a = tile symbol, b = suppressed repeats of the previous one */
  EV_NPC_TALK,        /* a = NpcId, b = DialogueId */
  EV_NOTEBOOK_OPEN,   /* a = number of entries */
  EV_KNOCKBACK,       /* x,y = where the player landed; a,b = the guarded tile */
  EV_ITEM_USE         /* a = ItemId, b = DialogueId, or D_NONE when nothing happened */
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
  Obs obs;
  NoteId notes[NOTE_LIMIT];
  int note_count;
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
void game_camera(const Game *g, int *x, int *y);
/* Map tile after applying overrides for what the player has observed. */
char game_tile(const Game *g, int map, int x, int y);
bool game_passable(const Game *g, int map, int x, int y);
bool game_knows(const Game *g, ObsId o);
/* Items the player owns, in display order; returns the count. */
int game_owned_items(const Game *g, ItemId *out);
int game_take_events(Game *g, GameEvent *out, int max);
#endif
