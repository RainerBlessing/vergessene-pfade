#include "audio.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                                         \
  do {                                                                                   \
    if (!(x)) {                                                                          \
      fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x);                               \
      return 1;                                                                          \
    }                                                                                    \
  } while (0)
static int playing(const Audio *a, SfxId id) {
  int n = 0;
  for (int i = 0; i < SFX_VOICES; i++)
    n += a->voices[i].position >= 0 && a->voices[i].id == id;
  return n;
}
static void silence(Audio *a) {
  for (int i = 0; i < SFX_VOICES; i++)
    a->voices[i].position = -1;
}
int main(void) {
  /* The sounds are generated, so the test is that each one has a body and that
   * the game's events reach the right one. No device is needed for either. */
  Audio a;
  audio_open(&a);
  for (int id = 0; id < SFX_COUNT; id++) {
    CHECK(a.length[id] > 0 && a.samples[id]);
    float peak = 0;
    for (int i = 0; i < a.length[id]; i++) {
      float v = a.samples[id][i] < 0 ? -a.samples[id][i] : a.samples[id][i];
      if (v > peak)
        peak = v;
    }
    CHECK(peak > 0.05f && peak <= 1.0f); /* audible, and no clipping */
    CHECK(a.length[id] <= SFX_RATE);     /* short: at most a second */
  }
  /* Events map to their sound. */
  const struct {
    GameEvent event;
    SfxId sound;
  } cases[] = {
      {{EV_OBSERVE, 0, 0, 0, OBS_TRACKS, 0}, SFX_WRITE},
      {{EV_STONE_PUSH, 0, 0, 0, STONE_START_X, STONE_START_Y - 1}, SFX_SCRAPE},
      {{EV_STONE_PUSH, 0, 0, 0, STONE_HOLLOW_X, STONE_HOLLOW_Y}, SFX_SETTLE},
      {{EV_ITEM_USE, 0, 0, 0, ITEM_HERB, D_I_TEND_FOX}, SFX_FOX},
      {{EV_MEND, 0, 0, 0, 1, 1}, SFX_CERAMIC},
      {{EV_STAKE, 0, 0, 0, 1, 0}, SFX_STAKE},
      {{EV_ENCOUNTER, 0, 0, 0, MOOD_ANGRY, 0}, SFX_CREAK},
      {{EV_ENCOUNTER_ACTION, 0, 0, 0, ENC_ATTACK, MOOD_ANGRY}, SFX_HIT},
      {{EV_OUTCOME, 0, 0, 0, OUT_FIGHT, 0}, SFX_BREAK},
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    silence(&a);
    audio_events(&a, &cases[i].event, 1);
    CHECK(playing(&a, cases[i].sound) == 1);
  }
  /* Quiet events stay quiet: walking about and failing to find anything. */
  const GameEvent quiet[] = {{EV_EXAMINE_NOTHING, 0, 0, 0, '.', 0},
                             {EV_NOTEBOOK_OPEN, 0, 0, 0, 3, 0},
                             {EV_PHASE, 0, 0, 0, PHASE_MORNING, 0},
                             {EV_ITEM_USE, 0, 0, 0, ITEM_HERB, D_NONE},
                             {EV_MEND, 0, 0, 0, 1, 0},
                             {EV_ENCOUNTER_ACTION, 0, 0, 0, ENC_WAIT, MOOD_WARY}};
  silence(&a);
  audio_events(&a, quiet, (int)(sizeof quiet / sizeof quiet[0]));
  for (int i = 0; i < SFX_VOICES; i++)
    CHECK(a.voices[i].position < 0);
  /* The setting turns them off and says so. */
  CHECK(strstr(audio_label(&a), "AN") != NULL);
  audio_cycle(&a);
  CHECK(strstr(audio_label(&a), "LEISE") != NULL);
  audio_cycle(&a);
  CHECK(strstr(audio_label(&a), "AUS") != NULL);
  audio_play(&a, SFX_CLICK);
  for (int i = 0; i < SFX_VOICES; i++)
    CHECK(a.voices[i].position < 0);
  audio_cycle(&a);
  CHECK(strstr(audio_label(&a), "AN") != NULL);
  audio_play(&a, SFX_CLICK);
  CHECK(playing(&a, SFX_CLICK) == 1);
  /* More sounds at once than voices: the extra ones are dropped, not queued. */
  for (int i = 0; i < SFX_VOICES + 4; i++)
    audio_play(&a, SFX_CLICK);
  CHECK(playing(&a, SFX_CLICK) == SFX_VOICES);
  audio_close(&a);
  puts("Sound effects are generated, mapped and switchable.");
  return 0;
}
