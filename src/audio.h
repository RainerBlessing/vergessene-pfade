#ifndef AUDIO_H
#define AUDIO_H
#include "game.h"
#include <SDL3/SDL.h>
/* Short sounds tied to single actions. No music, no typing. */
typedef enum {
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
#define SFX_VOICES 6
/* Loud enough beside a window; low enough that no single sound clips. */
#define AUDIO_GAIN 1.35f
#define SFX_RATE 22050
typedef struct {
  SDL_AudioStream *stream;
  float *samples[SFX_COUNT];
  int length[SFX_COUNT];
  struct {
    SfxId id;
    int position;
  } voices[SFX_VOICES];
  int level; /* 0 off, 1 quiet, 2 normal */
  bool left_foot;
} Audio;
void audio_open(Audio *a);
void audio_close(Audio *a);
void audio_play(Audio *a, SfxId id);
/* One footfall, alternating between the two. */
void audio_footstep(Audio *a);
/* Turn the game's own events into sounds. */
void audio_events(Audio *a, const GameEvent *events, int count);
/* Mix the voices that are playing into `out`, clamped. Used by audio_update. */
void audio_mix(Audio *a, float *out, int frames);
/* Keep the device fed; call once per frame. */
void audio_update(Audio *a);
void audio_cycle(Audio *a);
float audio_gain(const Audio *a);
const char *audio_label(const Audio *a);
#endif
