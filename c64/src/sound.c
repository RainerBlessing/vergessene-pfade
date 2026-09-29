/* SID effects: one voice, a few register writes per step, no sustain. The
 * envelope lets every step die away by itself, so nothing needs switching off. */
#include "sound.h"
#include <c64.h>
#include <stdint.h>

#define JIFFY (*(volatile uint8_t *)0xA2) /* the Kernal counts it in its own IRQ */

#define GATE 0x01
#define NOISE 0x80
#define PULSE 0x40
#define SAW 0x20
#define TRIANGLE 0x10
#define VOLUME 8 /* half of the maximum: a room's worth, not a wall's */
/* Register value for a PAL frequency in Hz (985248 Hz clock, 24-bit phase). */
#define HZ(h) ((uint16_t)((h) * 17L))

typedef struct {
  uint16_t freq;
  uint8_t wave; /* 0: a rest */
  uint8_t frames;
} Step;

#define END {0, 0, 0}
static const Step click[] = {{HZ(1400), PULSE, 2}, END};
static const Step step_a[] = {{HZ(180), NOISE, 2}, END};
static const Step step_b[] = {{HZ(260), NOISE, 2}, END};
static const Step write_[] = {{HZ(900), TRIANGLE, 2}, {HZ(1200), TRIANGLE, 3}, END};
static const Step scrape[] = {{HZ(140), NOISE, 4}, {HZ(110), NOISE, 4}, END};
static const Step settle[] = {{HZ(100), NOISE, 3}, {HZ(70), TRIANGLE, 5}, END};
static const Step fox[] = {
    {HZ(700), TRIANGLE, 3}, {HZ(1000), TRIANGLE, 3}, {HZ(800), TRIANGLE, 4}, END};
static const Step ceramic[] = {{HZ(2400), PULSE, 2}, {HZ(3100), PULSE, 4}, END};
static const Step stake[] = {{HZ(110), PULSE, 3}, {0, 0, 3}, {HZ(110), PULSE, 4}, END};
static const Step creak[] = {{HZ(90), SAW, 4}, {HZ(80), SAW, 4}, {HZ(65), SAW, 5}, END};
static const Step hit[] = {{HZ(250), NOISE, 4}, END};
static const Step break_[] = {
    {HZ(300), NOISE, 4}, {HZ(200), NOISE, 4}, {HZ(120), NOISE, 6}, END};

static const Step *const effects[SFX_COUNT] = {
    [SFX_CLICK] = click,  [SFX_STEP_A] = step_a,   [SFX_STEP_B] = step_b,
    [SFX_WRITE] = write_, [SFX_SCRAPE] = scrape,   [SFX_SETTLE] = settle,
    [SFX_FOX] = fox,      [SFX_CERAMIC] = ceramic, [SFX_STAKE] = stake,
    [SFX_CREAK] = creak,  [SFX_HIT] = hit,         [SFX_BREAK] = break_,
};

void sound_init(void) {
  SID.amp = VOLUME;
  SID.v1.pw = 0x0200; /* a narrow pulse: dry, like wood */
  SID.v1.ad = 0x08;   /* instant attack, 204 ms decay */
  SID.v1.sr = 0x00;
}

static void wait_frames(uint8_t n) {
  while (n--) {
    uint8_t now = JIFFY;
    while (JIFFY == now)
      ;
  }
}

void sound_play(SfxId id) {
  if (id <= SFX_NONE || id >= SFX_COUNT)
    return;
  for (const Step *s = effects[id]; s->frames; s++) {
    SID.v1.ctrl = 0; /* release, so the next gate retriggers */
    if (s->wave) {
      SID.v1.freq = s->freq;
      SID.v1.ctrl = (uint8_t)(s->wave | GATE);
    }
    wait_frames(s->frames);
  }
  SID.v1.ctrl = 0;
}
