#ifndef JOURNEY_H
#define JOURNEY_H
#include "game.h"
/* Observers may render and drain the event log; they must not change game state. */
typedef void (*JourneyObserver)(Game *g, const char *label, void *context);
bool journey(Game *g, JourneyObserver observer, void *context);
/* One journey per outcome; each explores first. */
bool journey_fight(Game *g, JourneyObserver observer, void *context);
#endif
