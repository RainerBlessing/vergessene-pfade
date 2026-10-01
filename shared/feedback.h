#ifndef SHARED_FEEDBACK_H
#define SHARED_FEEDBACK_H
/* What the game reports about itself, and how a sound follows from it. Included
 * after content.h (ITEM_HERB, D_NONE, ENC_ATTACK, OUT_FIGHT, STONE_HOLLOW_*).
 *
 * The PC keeps every event for the session log and the sound; the C64 keeps no
 * event, only the cue that follows from it (sfx_for_event). */

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
  EV_PHASE             /* a = Phase */
} EventType;

/* Short sounds tied to single actions: no music, no typing. One action leaves at
 * most one cue on the C64; when several apply, the later entry (the more
 * specific one) wins. */
typedef enum {
  SFX_NONE,
  SFX_CLICK,  /* confirming a line or a choice */
  SFX_STEP_A, /* two quiet footfalls, used alternately */
  SFX_STEP_B,
  SFX_WRITE,   /* a new observation goes into the notebook */
  SFX_SCRAPE,  /* the boundary stone moves */
  SFX_SETTLE,  /* and drops into its hollow */
  SFX_FOX,     /* bandage, then a small animal sound */
  SFX_CERAMIC, /* a shard finds its edge */
  SFX_STAKE,   /* two blows on wood */
  SFX_CREAK,   /* the kami rises */
  SFX_HIT,     /* a blow lands */
  SFX_BREAK,   /* the kami falls apart */
  SFX_COUNT
} SfxId;

/* The sound an event makes, or SFX_NONE. Not covered, because the two versions
 * trigger them differently: SFX_WRITE (PC: a new observation, C64: a new note),
 * and the click and the footfall (set by each frontend or action itself). */
static inline SfxId sfx_for_event(EventType type, int a, int b) {
  switch (type) {
  case EV_STONE_PUSH:
    return a == STONE_HOLLOW_X && b == STONE_HOLLOW_Y ? SFX_SETTLE : SFX_SCRAPE;
  case EV_ITEM_USE:
    return a == ITEM_HERB && b != D_NONE ? SFX_FOX : SFX_NONE;
  case EV_MEND:
    return b ? SFX_CERAMIC : SFX_NONE;
  case EV_STAKE:
    return SFX_STAKE;
  case EV_ENCOUNTER:
    return SFX_CREAK;
  case EV_ENCOUNTER_ACTION:
    return a == ENC_ATTACK ? SFX_HIT : SFX_NONE;
  case EV_OUTCOME:
    return a == OUT_FIGHT ? SFX_BREAK : SFX_NONE;
  default:
    return SFX_NONE;
  }
}
#endif
