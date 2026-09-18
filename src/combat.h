#ifndef COMBAT_H
#define COMBAT_H
#include "inventory.h"
typedef struct {
  const char *name;
  int hp, attack, defense;
} EnemyDef;
extern const EnemyDef kami;
typedef struct {
  int hp;
  bool won, lost;
  uint32_t random;
} Combat;
int combat_damage(int attack, int defense, int modifier);
void combat_begin(Combat *c);
/* One exchange. `rage` is added to the spirit's attack (an angry kami hits
 * harder). A failed herb costs no turn. */
void combat_turn(Combat *c, Player *p, int rage, bool herb, char *message, int size);
#endif
