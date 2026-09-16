#ifndef JOURNEY_H
#define JOURNEY_H
#include "game.h"
typedef void (*JourneyObserver)(const Game *g, const char *label,
                                void *context);
bool journey(Game *g, JourneyObserver observer, void *context);
#endif
