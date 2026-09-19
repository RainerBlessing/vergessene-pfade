#include "audio.h"
#include <stdlib.h>

/* Every sound is built here rather than shipped as a file: they are short,
 * and this keeps the game one binary plus its maps. Levels are set per sound
 * so that the frequent ones (click, write) stay well under the rare ones. */
static Uint32 noise_state = 22695477u;
static float noise(void) {
  noise_state = noise_state * 1664525u + 1013904223u;
  return (float)((int)((noise_state >> 16) & 0xffff) - 32768) / 32768.0f;
}
static float tone(float phase) { return SDL_sinf(phase * 6.2831853f); }
/* Exponential fall from 1 to 0 across `length`. */
static float fade(int i, int length, float steepness) {
  float t = (float)i / (float)length;
  return SDL_expf(-steepness * t) * (1.0f - t);
}
static float *make(Audio *a, SfxId id, int milliseconds) {
  int n = SFX_RATE * milliseconds / 1000;
  a->samples[id] = calloc((size_t)n, sizeof(float));
  a->length[id] = a->samples[id] ? n : 0;
  return a->samples[id];
}
static void build(Audio *a) {
  float *s;
  int n;
  /* A dry click: a very short noise tick with a wooden edge. */
  if ((s = make(a, SFX_CLICK, 30))) {
    n = a->length[SFX_CLICK];
    for (int i = 0; i < n; i++)
      s[i] =
          0.25f * fade(i, n, 26.0f) * (noise() * 0.6f + tone((float)i * 900 / SFX_RATE));
  }
  /* Two footfalls, quiet and slightly different from each other so that
   * walking on does not turn into a machine. */
  for (int step = 0; step < 2; step++) {
    SfxId id = step ? SFX_STEP_B : SFX_STEP_A;
    if (!(s = make(a, id, 90)))
      continue;
    n = a->length[id];
    float low = 0;
    for (int i = 0; i < n; i++) {
      low = low * 0.72f + noise() * 0.28f;
      s[i] = (step ? 0.075f : 0.085f) * fade(i, n, 18.0f) *
             (low * 2.0f + 0.3f * tone((float)i * (step ? 120 : 150) / SFX_RATE));
    }
  }
  /* Pen on paper: brushed noise in two strokes. */
  if ((s = make(a, SFX_WRITE, 220))) {
    n = a->length[SFX_WRITE];
    for (int i = 0; i < n; i++) {
      float stroke = 0.5f + 0.5f * tone((float)i * 7.0f / SFX_RATE);
      s[i] = 0.14f * fade(i, n, 2.0f) * stroke * noise();
    }
  }
  /* Stone over earth: low, grainy, slow to start. */
  if ((s = make(a, SFX_SCRAPE, 420))) {
    n = a->length[SFX_SCRAPE];
    float low = 0;
    int attack = n / 5;
    for (int i = 0; i < n; i++) {
      low = low * 0.86f + noise() * 0.14f; /* a crude low pass */
      float swell = i < attack ? (float)i / (float)attack : fade(i - attack, n, 1.6f);
      s[i] = 0.5f * swell * (low * 2.4f + 0.2f * tone((float)i * 70 / SFX_RATE));
    }
  }
  /* And the dull end when it drops home. */
  if ((s = make(a, SFX_SETTLE, 260))) {
    n = a->length[SFX_SETTLE];
    for (int i = 0; i < n; i++)
      s[i] = 0.45f * fade(i, n, 7.0f) *
             (tone((float)i * 74 / SFX_RATE) + 0.5f * tone((float)i * 110 / SFX_RATE) +
              0.3f * noise());
  }
  /* Bandage rustle, then a small yip. */
  if ((s = make(a, SFX_FOX, 520))) {
    n = a->length[SFX_FOX];
    int yip = n * 3 / 5;
    for (int i = 0; i < n; i++) {
      if (i < yip) {
        float r = 0.5f + 0.5f * tone((float)i * 11.0f / SFX_RATE);
        s[i] = 0.12f * fade(i, yip, 1.2f) * r * noise();
      } else {
        int k = i - yip;
        float glide = 780.0f - 260.0f * (float)k / (float)(n - yip);
        s[i] = 0.2f * fade(k, n - yip, 5.0f) * tone((float)k * glide / SFX_RATE);
      }
    }
  }
  /* Ceramic: a small bright click with a ring. */
  if ((s = make(a, SFX_CERAMIC, 220))) {
    n = a->length[SFX_CERAMIC];
    for (int i = 0; i < n; i++)
      s[i] = 0.22f * fade(i, n, 12.0f) *
             (tone((float)i * 1560 / SFX_RATE) + 0.4f * tone((float)i * 2350 / SFX_RATE));
  }
  /* Two blows on a wooden stake. */
  if ((s = make(a, SFX_STAKE, 520))) {
    n = a->length[SFX_STAKE];
    int second = n / 2;
    for (int i = 0; i < n; i++) {
      int k = i < second ? i : i - second;
      int span = second;
      s[i] = 0.4f * fade(k, span, 16.0f) *
             (tone((float)k * 190 / SFX_RATE) + 0.5f * noise());
    }
  }
  /* Wood under strain: a slow creak. */
  if ((s = make(a, SFX_CREAK, 700))) {
    n = a->length[SFX_CREAK];
    float phase = 0;
    int attack = n / 6;
    for (int i = 0; i < n; i++) {
      float wobble = 118.0f + 26.0f * tone((float)i * 5.5f / SFX_RATE);
      phase += wobble / SFX_RATE;
      float shape = i < attack ? (float)i / (float)attack : fade(i - attack, n, 2.2f);
      s[i] = 0.34f * shape * (tone(phase) * 0.8f + 0.25f * noise());
    }
  }
  /* A blow landing. */
  if ((s = make(a, SFX_HIT, 180))) {
    n = a->length[SFX_HIT];
    for (int i = 0; i < n; i++)
      s[i] =
          0.4f * fade(i, n, 14.0f) * (noise() * 0.7f + tone((float)i * 130 / SFX_RATE));
  }
  /* Dry breaking, falling away. */
  if ((s = make(a, SFX_BREAK, 620))) {
    n = a->length[SFX_BREAK];
    for (int i = 0; i < n; i++) {
      float crackle = noise() * (noise() > 0.4f ? 1.0f : 0.25f);
      float pitch = 300.0f - 200.0f * (float)i / (float)n;
      s[i] =
          0.36f * fade(i, n, 3.0f) * (crackle + 0.5f * tone((float)i * pitch / SFX_RATE));
    }
  }
}
void audio_open(Audio *a) {
  SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, SFX_RATE};
  SDL_memset(a, 0, sizeof *a);
  a->level = 2;
  for (int i = 0; i < SFX_VOICES; i++)
    a->voices[i].position = -1;
  build(a);
  a->stream =
      SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
  if (a->stream)
    SDL_ResumeAudioStreamDevice(a->stream);
  else /* no sound card is no reason to stop playing */
    SDL_Log("Keine Klangeffekte: %s", SDL_GetError());
}
void audio_close(Audio *a) {
  if (a->stream)
    SDL_DestroyAudioStream(a->stream);
  for (int i = 0; i < SFX_COUNT; i++)
    free(a->samples[i]);
  SDL_memset(a, 0, sizeof *a);
}
void audio_play(Audio *a, SfxId id) {
  if (!a->level || id < 0 || id >= SFX_COUNT || !a->length[id])
    return;
  for (int i = 0; i < SFX_VOICES; i++)
    if (a->voices[i].position < 0) {
      a->voices[i].id = id;
      a->voices[i].position = 0;
      return;
    }
}
void audio_events(Audio *a, const GameEvent *events, int count) {
  for (int i = 0; i < count; i++) {
    const GameEvent *e = &events[i];
    switch (e->type) {
    case EV_STEP:
      a->left_foot = !a->left_foot;
      audio_play(a, a->left_foot ? SFX_STEP_A : SFX_STEP_B);
      break;
    case EV_OBSERVE:
      audio_play(a, SFX_WRITE);
      break;
    case EV_STONE_PUSH:
      audio_play(a, e->a == STONE_HOLLOW_X && e->b == STONE_HOLLOW_Y ? SFX_SETTLE
                                                                     : SFX_SCRAPE);
      break;
    case EV_ITEM_USE:
      if (e->a == ITEM_HERB && e->b != D_NONE)
        audio_play(a, SFX_FOX);
      break;
    case EV_MEND:
      if (e->b)
        audio_play(a, SFX_CERAMIC);
      break;
    case EV_STAKE:
      audio_play(a, SFX_STAKE);
      break;
    case EV_ENCOUNTER:
      audio_play(a, SFX_CREAK);
      break;
    case EV_ENCOUNTER_ACTION:
      if (e->a == ENC_ATTACK)
        audio_play(a, SFX_HIT);
      break;
    case EV_OUTCOME:
      if (e->a == OUT_FIGHT)
        audio_play(a, SFX_BREAK);
      break;
    default:
      break;
    }
  }
}
void audio_mix(Audio *a, float *out, int frames) {
  /* Loud enough to sit next to the game window; the clamp keeps two sounds at
   * once from tearing. */
  float volume = a->level == 2 ? 2.2f : a->level == 1 ? 1.0f : 0.0f;
  for (int i = 0; i < frames; i++)
    out[i] = 0;
  for (int v = 0; v < SFX_VOICES; v++) {
    int position = a->voices[v].position;
    if (position < 0)
      continue;
    SfxId id = a->voices[v].id;
    for (int i = 0; i < frames && position < a->length[id]; i++, position++)
      out[i] += a->samples[id][position] * volume;
    a->voices[v].position = position < a->length[id] ? position : -1;
  }
  for (int i = 0; i < frames; i++)
    out[i] = out[i] > 1.0f ? 1.0f : out[i] < -1.0f ? -1.0f : out[i];
}
void audio_update(Audio *a) {
  float out[512];
  if (!a->stream)
    return;
  /* Keep about a tenth of a second queued: short enough to stay responsive. */
  if (SDL_GetAudioStreamQueued(a->stream) > (int)sizeof(float) * SFX_RATE / 10)
    return;
  audio_mix(a, out, (int)(sizeof out / sizeof out[0]));
  SDL_PutAudioStreamData(a->stream, out, (int)sizeof out);
}
/* An, leise, aus, und wieder an. */
void audio_cycle(Audio *a) { a->level = (a->level + 2) % 3; }
const char *audio_label(const Audio *a) {
  return a->level == 2   ? "F3 KLANG: AN"
         : a->level == 1 ? "F3 KLANG: LEISE"
                         : "F3 KLANG: AUS";
}
