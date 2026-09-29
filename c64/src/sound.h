#ifndef SOUND_H
#define SOUND_H
#include "game.h"

void sound_init(void);
/* Plays one cue on voice 1 and returns when it has died away. The game runs
 * without an interrupt of its own, so an effect stays short enough to run
 * here in the main loop; keys pressed meanwhile wait in the Kernal's buffer. */
void sound_play(SfxId id);
#endif
