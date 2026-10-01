#ifndef AUDIO_H
#define AUDIO_H
#include "game.h"
#include <SDL3/SDL.h>
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
