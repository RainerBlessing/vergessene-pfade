#ifndef SHARED_STATE_H
#define SHARED_STATE_H
/* The part of Game that PC and C64 keep alike, so a rule written once finds the
 * same fields on both. Each version declares, before the struct:
 *   Coord  a signed number small enough for a map position or direction
 *          (PC: int, C64: int8_t)
 *   Count  an unsigned counter or index (PC: int, C64: uint8_t)
 * and defines NOTE_LIMIT and MESSAGE_LIMIT, and includes content.h for Obs.
 * Fields whose type is an enum on the PC and a byte on the C64 (state, mood,
 * outcome, opens) stay in each version's own struct under the same name. */
#define GAME_CORE_FIELDS                                                                 \
  Coord x, y, dx, dy;                                                                    \
  /* The speaker: an NPC, SPEAKER_SCENE, or -1 with the examined tile symbol            \
   * (0 for items). */                                                                   \
  Coord npc;                                                                             \
  char examined;                                                                         \
  const char *scene; /* panel title while npc == SPEAKER_SCENE */                       \
  const char *place; /* the room the player just stepped into */                         \
  Count page, selection, dialogue, scroll;                                               \
  Count place_ticks; /* how much longer the room's name is shown */                      \
  Count note_ticks;  /* how much longer the fresh-entry notice is shown */               \
  Obs obs;                                                                               \
  uint8_t notes[NOTE_LIMIT];                                                             \
  Count note_count;                                                                      \
  Coord stone_x, stone_y; /* the boundary stone the loggers moved */                     \
  Count mend_placed;      /* pieces of the bowl already set */                           \
  Coord daigo_x, daigo_y; /* the foreman walks along while staking the boundary */       \
  bool daigo_follows;                                                                    \
  uint8_t staked; /* bit per stake already driven in */                                  \
  /* The direction last blocked: the second try the same way says what is in the        \
   * way (#20). */                                                                       \
  Coord blocked_dx, blocked_dy;                                                          \
  char message[MESSAGE_LIMIT];
#endif
