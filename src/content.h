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
  OBS_KAMI_SEEN,
  OBS_KAMI_ANGERED,
  OBS_STONE_MOVED,
  OBS_BOWL_DRYING,
  OBS_BOWL_READY,
  OBS_KAMI_CALMED,
  OBS_DAIGO_DEAL,
  OBS_SETTLED,
  OBS_GREY_TRACE,
  OBS_MORNING,
  OBS_TEASED,
  OBS_COUNT
} ObsId;
typedef enum { OUT_NONE, OUT_FIGHT, OUT_BOUNDARY, OUT_MEND, OUTCOME_COUNT } Outcome;
/* In a rule: applies whatever has been decided. OUT_NONE means "not yet decided". */
#define OUT_ANY OUTCOME_COUNT
extern const char *const outcome_names[OUTCOME_COUNT];
/* Time: the night at the inn separates the immediate change from the later one. */
typedef enum { PHASE_BEFORE, PHASE_MORNING, PHASE_COUNT } Phase;
#define PHASE_ANY PHASE_COUNT
extern const char *const phase_names[PHASE_COUNT];
/* Every outcome must show a gain and a loss, in both phases (checked by a test). */
typedef enum { TAG_NEUTRAL, TAG_GAIN, TAG_LOSS } ChangeTag;
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
  D_MIO_HERB_MORE,
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
  D_X_DEN_EMPTY,
  D_X_TRACKS,
  D_I_TEND_FOX,
  D_ENC_APPEAR,
  D_ENC_WAIT_ANGRY,
  D_ENC_WAIT_WARY,
  D_ENC_WAIT_CALM,
  D_ENC_ATTACK,
  D_ENC_RETREAT,
  D_ENC_OFFER_SHARDS,
  D_ENC_VICTORY,
  D_ENC_DEFEAT,
  D_SUMI_FOUGHT,
  D_SUMI_BOUNDARY,
  D_DAIGO_FOUGHT,
  D_DAIGO_BOUNDARY,
  D_X_STONE_STUCK,
  D_SCENE_BOUNDARY,
  D_ORIHA_MEND,
  D_ORIHA_DRYING,
  D_ORIHA_READY,
  D_ORIHA_AFTER,
  D_MEND_WRONG,
  D_MEND_DONE,
  D_ENC_OFFER_BOWL,
  D_DAIGO_OFFER,
  D_DAIGO_WALKING,
  D_DAIGO_MEND,
  D_SUMI_MEND,
  D_STAKE_SET,
  D_SCENE_MEND,
  D_X_INN_SIGN,
  D_X_WORKSHOP_SIGN,
  D_X_WORKBENCH,
  D_X_FUTON,
  D_X_FUTON_AWAKE,
  D_X_FUTON_MORNING,
  D_PROMPT_SLEEP,
  D_SCENE_MORNING,
  D_X_TRACE,
  D_X_TRACE_TRACKS,
  D_X_INSCRIPTION_LATE,
  D_SCENE_TEASER,
  D_SUMI_MORNING_FIGHT,
  D_SUMI_MORNING_BOUNDARY,
  D_SUMI_MORNING_MEND,
  D_MIO_MORNING_FIGHT,
  D_MIO_MORNING_BOUNDARY,
  D_MIO_MORNING_MEND,
  D_DAIGO_MORNING_FIGHT,
  D_DAIGO_MORNING_BOUNDARY,
  D_DAIGO_MORNING_MEND,
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
  N_FOX_GONE,
  N_TRACKS,
  N_KAMI,
  N_KAMI_SHARDS,
  N_FOUGHT,
  N_BOUNDARY,
  N_MENDED,
  N_BOWL_READY,
  N_KAMI_CALM,
  N_DEAL,
  N_MEND,
  N_MORNING,
  N_TRACE,
  N_VISITOR,
  NOTE_COUNT
} NoteId;
extern const char *const notes[NOTE_COUNT];

/* First matching rule wins. */
/* What a conversation opens once its last page is read. */
typedef enum { OPEN_NOTHING, OPEN_MEND, OPEN_FOLLOW, OPEN_NIGHT } DialogueOpens;
typedef struct {
  NpcId npc;
  Obs needs, forbids, grants;
  DialogueId dialogue;
  NoteId note;
  ItemId gives;
  Outcome outcome; /* OUT_ANY matches whatever has been decided */
  DialogueOpens opens;
  Phase phase; /* PHASE_ANY matches any time */
} DialogueRule;
extern const DialogueRule dialogue_rules[];
extern const int dialogue_rule_count;

typedef enum { POINT_AT, POINT_SYMBOL, POINT_ITEM, POINT_STONE } PointKind;
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

/* The first override that fits replaces the map tile: observations, the outcome
 * (OUT_ANY: any) and the phase (PHASE_ANY: any) all have to match. */
typedef struct {
  int map, x, y;
  char symbol;
  Obs needs;
  Outcome outcome;
  Phase phase;
  ChangeTag tag;
} TileOverride;
extern const TileOverride tile_overrides[];
extern const int tile_override_count;
/* What the outcomes change in the world, tagged as gain or loss. */
extern const TileOverride outcome_changes[];
extern const int outcome_change_count;

/* Encounter: actions change the spirit's mood, not only its health. */
typedef enum { MOOD_ANGRY, MOOD_WARY, MOOD_CALM, MOOD_COUNT } Mood;
extern const char *const mood_names[MOOD_COUNT];
typedef enum {
  ENC_WAIT,
  ENC_OFFER,
  ENC_ATTACK,
  ENC_HEAL,
  ENC_RETREAT,
  ENC_COUNT
} EncounterAction;
/* When an option is offered: before the fight, during it, or both. */
typedef enum { OPT_PEACE, OPT_FIGHT, OPT_BOTH } OptionWhen;
extern const char *const encounter_action_names[ENC_COUNT];
/* Mood after an action, by action and current mood. */
extern const Mood encounter_transitions[ENC_COUNT][MOOD_COUNT];
/* What the player may try. Offering appears only with something to offer. */
typedef struct {
  EncounterAction action;
  const char *label;
  Obs needs;
  OptionWhen when;
} EncounterOption;
#define ENCOUNTER_OPTION_LIMIT 8 /* buffer size for game_encounter_options */
extern const EncounterOption encounter_options[];
extern const int encounter_option_count;
typedef struct {
  ItemId item;
  Obs needs, grants;
  Mood result;
  DialogueId dialogue;
  NoteId note;
} EncounterOffer;
extern const EncounterOffer encounter_offers[];
extern const int encounter_offer_count;
/* Text after an action, by action and the mood before it. */
extern const DialogueId encounter_lines[ENC_COUNT][MOOD_COUNT];

/* The one stone the loggers moved: start, and where it belongs. */
#define STONE_START_X 24
#define STONE_START_Y 16
#define STONE_HOLLOW_X 24
#define STONE_HOLLOW_Y 12

/* Rooms that name themselves for a moment when the player steps inside. */
typedef struct {
  int map, x, y, width, height;
  const char *name;
  Obs needs;
} Place;
extern const Place places[];
extern const int place_count;
#define PLACE_TICKS 14 /* actions the name stays on screen */

/* The two answers to the question about the night, in this order. */
#define NIGHT_CHOICES 2
extern const char *const night_choices[NIGHT_CHOICES];
/* Where the inn stands: both ways into the night end up here. */
#define INN_X 4
#define INN_Y 14

/* Where the new boundary is staked out, along the animal tracks. */
#define STAKE_COUNT 3
typedef struct {
  int x, y;
} StakeSpot;
extern const StakeSpot stakes[STAKE_COUNT];

/* Kintsugi: the pieces in the order they go back, each with the gap it fills. */
#define MEND_PIECES 4
typedef struct {
  const char *shard, *gap;
} MendPiece;
extern const MendPiece mend_pieces[MEND_PIECES];
/* The order the pieces are listed in, so the right one is not simply first. */
extern const int mend_display[MEND_PIECES];

#define SPEAKER_SCENE (-2)
#endif
