#ifndef CONTENT_H
#define CONTENT_H
#include "inventory.h"
#include <stdint.h>
enum { MAP_VILLAGE, MAP_FOREST };

/* Observations only; the game never stores conclusions (design rule 1). */
typedef enum {
  OBS_ASKED_BY_SUMI,
  OBS_KENTA_STARE,
  OBS_CLAW_MARKS_EDGE,
  OBS_FRESH_STUMPS,
  OBS_BROKEN_ROPE,
  OBS_SHRINE_INSCRIPTION,
  OBS_STONE_DRAGGED,
  OBS_STONE_HOLLOW,
  OBS_BOWL_SHARDS,
  OBS_BOWL_MARK,
  OBS_HOUSE_MARK,
  OBS_BOWL_OWNER,
  OBS_LEDGER_DEBT,
  OBS_FOX_WOUNDED,
  OBS_MIO_HERB,
  OBS_FOX_TENDED, /* also the ability "Tierzeichen": tracks become visible */
  OBS_TRACKS,
  OBS_COUNT
} ObsId;
typedef uint32_t Obs;
#define OBS(o) ((Obs)1 << (o))
extern const char *const obs_names[OBS_COUNT];

typedef enum { NPC_SUMI, NPC_ORIHA, NPC_MIO, NPC_KENTA, NPC_DAIGO, NPC_COUNT } NpcId;
typedef struct {
  int map, x, y, sprite;
  const char *key, *name;
} Npc;
extern const Npc npcs[NPC_COUNT];

#define TRANSITION_COUNT 2
typedef struct {
  int map, x, y, to_map, to_x, to_y;
} Transition;
extern const Transition transitions[TRANSITION_COUNT];

typedef enum {
  D_NONE,
  D_SCENE_ARRIVAL,
  D_SUMI_TASK,
  D_SUMI_WAITING,
  D_SUMI_OWNER,
  D_SUMI_OWNER_KNOWN,
  D_ORIHA,
  D_ORIHA_SHARDS,
  D_ORIHA_OWNER,
  D_MIO,
  D_MIO_HERB,
  D_MIO_HERB_AGAIN,
  D_MIO_THANKS,
  D_KENTA,
  D_KENTA_AGAIN,
  D_DAIGO,
  D_DAIGO_LEDGER,
  D_X_WOOD,
  D_X_HOUSE_MARK,
  D_X_HOUSE_MARK_MATCH,
  D_X_CLAW,
  D_X_DRAGGED,
  D_X_HOLLOW,
  D_X_STUMPS,
  D_X_ROPE,
  D_X_INSCRIPTION,
  D_X_ALTAR_EMPTY,
  D_X_SHARDS,
  D_X_LEDGER,
  D_X_TENT,
  D_X_FOX,
  D_X_FOX_TENDED,
  D_X_TRACKS,
  D_I_TEND_FOX,
  D_I_BOWL_MARK,
  D_I_BOWL_MARK_MATCH,
  DIALOGUE_COUNT
} DialogueId;
#define DIALOGUE_PAGES 3
typedef struct {
  int count;
  const char *pages[DIALOGUE_PAGES];
} Dialogue;
extern const Dialogue dialogues[DIALOGUE_COUNT];

/* Notebook entries are diegetic: no counters, checkmarks or conclusions. */
typedef enum {
  NOTE_NONE,
  N_ASKED,
  N_KENTA,
  N_CLAW,
  N_DRAGGED,
  N_HOLLOW,
  N_STUMPS,
  N_ROPE,
  N_INSCRIPTION,
  N_SHARDS,
  N_BOWL_MARK,
  N_HOUSE_MARK,
  N_MARKS_MATCH,
  N_OWNER,
  N_LEDGER,
  N_FOX,
  N_HERB,
  N_FOX_TENDED,
  N_TRACKS,
  NOTE_COUNT
} NoteId;
extern const char *const notes[NOTE_COUNT];

/* First matching rule wins. */
typedef struct {
  NpcId npc;
  Obs needs, forbids, grants;
  DialogueId dialogue;
  NoteId note;
  ItemId gives;
} DialogueRule;
extern const DialogueRule dialogue_rules[];
extern const int dialogue_rule_count;

typedef enum { POINT_AT, POINT_SYMBOL, POINT_ITEM } PointKind;
/* First matching point wins: POINT_AT by map position, POINT_SYMBOL by tile
 * symbol on a map, POINT_ITEM by an inventory item. An item point with a symbol
 * applies only while facing that tile on its map, and consumes `takes`. */
typedef struct {
  PointKind kind;
  int map, x, y;
  char symbol;
  ItemId item, gives, takes;
  Obs needs, grants;
  DialogueId dialogue;
  NoteId note;
} ExaminePoint;
extern const ExaminePoint examine_points[];
extern const int examine_point_count;

/* The first override whose observations are known replaces the map tile. */
typedef struct {
  int map, x, y;
  char symbol;
  Obs needs;
} TileOverride;
extern const TileOverride tile_overrides[];
extern const int tile_override_count;

#define SPEAKER_SCENE (-2)
#endif
