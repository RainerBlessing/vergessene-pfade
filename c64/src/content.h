#ifndef CONTENT_H
#define CONTENT_H
#include "world.h"

/* Observations only; the game never stores conclusions (design rule 1).
 * This is the subset the C64 POC uses: exploration and the encounter. */
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
  OBS_KAMI_SEEN,
  OBS_KAMI_ANGERED,
  OBS_STONE_MOVED, /* the stone stands neither where it was dragged to nor home */
  OBS_COUNT
} ObsId;
typedef uint32_t Obs;
#define OBS(o) ((Obs)1 << (o))

typedef enum { OUT_NONE, OUT_FIGHT, OUT_BOUNDARY, OUTCOME_COUNT } Outcome;
/* In a rule: applies whatever has been decided. */
#define OUT_ANY OUTCOME_COUNT

typedef enum { ITEM_NONE, ITEM_HERB, ITEM_SHARDS, ITEM_COUNT } ItemId;
typedef struct {
  const char *name;
  uint8_t heal;
} ItemDef;
extern const ItemDef items[ITEM_COUNT];

typedef enum { NPC_SUMI, NPC_ORIHA, NPC_MIO, NPC_KENTA, NPC_DAIGO, NPC_COUNT } NpcId;
typedef struct {
  uint8_t map, x, y;
  char glyph;
  const char *name;
} Npc;
extern const Npc npcs[NPC_COUNT];

#define TRANSITION_COUNT 2
typedef struct {
  uint8_t map, x, y, to_map, to_x, to_y;
} Transition;
extern const Transition transitions[TRANSITION_COUNT];

typedef enum {
  D_NONE,
  D_SCENE_ARRIVAL,
  D_SUMI_TASK,
  D_SUMI_WAITING,
  D_SUMI_OWNER,
  D_SUMI_OWNER_KNOWN,
  D_SUMI_FOUGHT,
  D_SUMI_BOUNDARY,
  D_ORIHA,
  D_ORIHA_SHARDS,
  D_ORIHA_OWNER,
  D_MIO,
  D_MIO_HERB,
  D_MIO_HERB_MORE,
  D_MIO_HERB_AGAIN,
  D_MIO_THANKS,
  D_KENTA,
  D_KENTA_AGAIN,
  D_DAIGO,
  D_DAIGO_LEDGER,
  D_DAIGO_FOUGHT,
  D_DAIGO_BOUNDARY,
  D_X_WOOD,
  D_X_HOUSE_MARK,
  D_X_HOUSE_MARK_MATCH,
  D_X_CLAW,
  D_X_DRAGGED,
  D_X_STONE_STUCK,
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
  D_X_INN_SIGN,
  D_X_WORKSHOP_SIGN,
  D_X_WORKBENCH,
  D_I_TEND_FOX,
  D_I_BOWL_MARK,
  D_I_BOWL_MARK_MATCH,
  D_ENC_APPEAR,
  D_ENC_WAIT_ANGRY,
  D_ENC_WAIT_WARY,
  D_ENC_WAIT_CALM,
  D_ENC_ATTACK,
  D_ENC_RETREAT,
  D_ENC_OFFER_SHARDS,
  D_ENC_VICTORY,
  D_ENC_DEFEAT,
  D_SCENE_BOUNDARY,
  DIALOGUE_COUNT
} DialogueId;
#define DIALOGUE_PAGES 3
typedef struct {
  uint8_t count;
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
  N_KAMI,
  N_KAMI_SHARDS,
  N_FOUGHT,
  N_BOUNDARY,
  NOTE_COUNT
} NoteId;
extern const char *const notes[NOTE_COUNT];

/* First matching rule wins. */
typedef struct {
  uint8_t npc;
  Obs needs, forbids, grants;
  uint8_t dialogue, note, gives, outcome; /* outcome OUT_ANY: whatever was decided */
} DialogueRule;
extern const DialogueRule dialogue_rules[];
extern const uint8_t dialogue_rule_count;

typedef enum { POINT_AT, POINT_SYMBOL, POINT_ITEM, POINT_STONE } PointKind;
/* First matching point wins: POINT_AT by map position, POINT_SYMBOL by tile
 * symbol on a map, POINT_ITEM by an item in the bag. An item point with a
 * symbol applies only while facing that tile on its map, and consumes `takes`.
 * POINT_STONE is the one stone the loggers moved, wherever it stands. */
typedef struct {
  uint8_t kind, map, x, y;
  char symbol;
  uint8_t item, gives, takes;
  Obs needs, grants;
  uint8_t dialogue, note;
} ExaminePoint;
extern const ExaminePoint examine_points[];
extern const uint8_t examine_point_count;

/* The first override that fits replaces the map tile. */
typedef struct {
  uint8_t map, x, y;
  char symbol;
  Obs needs;
  uint8_t outcome;
} TileOverride;
extern const TileOverride tile_overrides[];
extern const uint8_t tile_override_count;

/* Encounter: actions change the spirit's mood, not only its health. */
typedef enum { MOOD_ANGRY, MOOD_WARY, MOOD_CALM, MOOD_COUNT } Mood;
extern const char *const mood_names[MOOD_COUNT];
typedef enum { ENC_WAIT, ENC_OFFER, ENC_ATTACK, ENC_HEAL, ENC_RETREAT, ENC_COUNT } EncounterAction;
typedef enum { OPT_PEACE, OPT_FIGHT, OPT_BOTH } OptionWhen;
extern const uint8_t encounter_transitions[ENC_COUNT][MOOD_COUNT];
typedef struct {
  uint8_t action;
  const char *label;
  uint8_t when;
} EncounterOption;
#define ENCOUNTER_OPTION_LIMIT 8
extern const EncounterOption encounter_options[];
extern const uint8_t encounter_option_count;
typedef struct {
  uint8_t item;
  Obs grants;
  uint8_t result, dialogue, note;
} EncounterOffer;
extern const EncounterOffer encounter_offers[];
extern const uint8_t encounter_offer_count;
extern const uint8_t encounter_lines[ENC_COUNT][MOOD_COUNT];

/* The stone the loggers moved: where they dragged it to, and where it belongs. */
#define STONE_START_X 24
#define STONE_START_Y 16
#define STONE_HOLLOW_X 24
#define STONE_HOLLOW_Y 12

/* Rooms that name themselves for a moment when the player steps inside. */
typedef struct {
  uint8_t map, x, y, width, height;
  const char *name;
  Obs needs;
} Place;
extern const Place places[];
extern const uint8_t place_count;
#define PLACE_TICKS 14 /* actions the name stays on screen */
#define NOTE_TICKS 14

#define TITLE_LINES 15
extern const char *const title_page[TITLE_LINES];

/* The spirit's numbers; the player's are in game.h. */
#define KAMI_HP 24
#define KAMI_ATTACK 6
#define KAMI_DEFENSE 2

#define SPEAKER_SCENE (-2)
#endif
