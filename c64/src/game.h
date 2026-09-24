#ifndef GAME_H
#define GAME_H
#include "content.h"

typedef enum {
  ACT_NONE,
  ACT_UP,
  ACT_DOWN,
  ACT_LEFT,
  ACT_RIGHT,
  ACT_CONFIRM,
  ACT_CANCEL,
  ACT_INVENTORY
} Action;

typedef enum {
  GAME_TITLE,
  GAME_EXPLORATION,
  GAME_DIALOGUE,
  GAME_INVENTORY,
  GAME_NOTEBOOK,
  GAME_ENCOUNTER,
  GAME_MEND,
  GAME_END /* die Schlusstafel des Ausschnitts */
} GameState;

#define NOTE_LIMIT 24
#define NOTES_PER_PAGE 3
#define MESSAGE_LIMIT 80
#define PLAYER_HP 24
#define PLAYER_ATTACK 8
#define PLAYER_DEFENSE 2

typedef struct {
  uint8_t map, state;
  int8_t x, y, dx, dy;
  int8_t npc;   /* the speaker: an NPC, SPEAKER_SCENE, or -1 for a tile */
  char examined; /* the tile the last text belongs to */
  const char *scene;
  const char *place;
  uint8_t page, selection, dialogue, scroll;
  uint8_t place_ticks, note_ticks;
  Obs obs;
  uint8_t notes[NOTE_LIMIT], note_count;
  uint8_t mood, outcome, fighting;
  int8_t stone_x, stone_y; /* the boundary stone the loggers moved */
  uint8_t mend_placed;     /* pieces of the bowl already set */
  int8_t daigo_x, daigo_y; /* the foreman walks along while staking the boundary */
  bool daigo_follows;
  uint8_t staked;          /* bit per stake already driven in */
  uint8_t opens;           /* what the open conversation leads to */
  /* Die zuletzt versperrte Richtung: Beim zweiten Versuch in dieselbe Richtung
   * sagt das Spiel, was im Weg steht (#20). */
  int8_t blocked_dx, blocked_dy;
  bool ended; /* die Schlusstafel kommt einmal */
  uint8_t bag[ITEM_COUNT];
  int16_t hp, kami_hp;
  uint32_t random;
  char message[MESSAGE_LIMIT];
} Game;

void game_init(Game *g);
void game_action(Game *g, Action a);
int8_t game_npc_at(const Game *g, int8_t x, int8_t y);
void game_camera(const Game *g, uint8_t view_w, uint8_t view_h, int8_t *x, int8_t *y);
/* Map tile after applying the overrides that fit what the player knows. */
char game_tile(const Game *g, uint8_t map, int8_t x, int8_t y);
bool game_passable(const Game *g, uint8_t map, int8_t x, int8_t y);
bool game_knows(const Game *g, ObsId o);
/* Does this override apply right now? The map view draws the base map first
 * and puts the overrides that fit on top, instead of asking per cell. */
bool game_shows(const Game *g, const TileOverride *o);
/* May the player push the boundary stone? Observations unlock it, not the plot. */
bool game_can_push(const Game *g);
/* Where a person stands; the foreman moves while he follows. */
void game_npc_pos(const Game *g, uint8_t npc, int8_t *x, int8_t *y);
/* Pieces still lying beside the bowl, in display order; returns the count. */
uint8_t game_mend_pieces(const Game *g, uint8_t *out);
/* Items the player owns, in display order; returns the count. */
uint8_t game_owned_items(const Game *g, uint8_t *out);
/* Indices into encounter_options offered now; `out` holds ENCOUNTER_OPTION_LIMIT. */
uint8_t game_encounter_options(const Game *g, uint8_t *out);
#endif
