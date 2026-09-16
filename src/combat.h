#ifndef COMBAT_H
#define COMBAT_H
#include "inventory.h"
typedef struct {
  const char *name;
  int hp, attack, defense, map, x, y;
} EnemyDef;
extern const EnemyDef beast;
typedef struct {
  int hp;
  bool won, lost;
  uint32_t random;
} Combat;
int combat_damage(int attack, int defense, int modifier);
void combat_begin(Combat *c);
void combat_turn(Combat *c, Player *p, bool herb, char *message, int size);
#endif
