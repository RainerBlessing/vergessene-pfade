#include "combat.h"
#include <stdio.h>
const EnemyDef kami = {"Waldkami", 24, 6, 2};
int combat_damage(int attack, int defense, int modifier) {
  int d = attack - defense + modifier;
  return d < 1 ? 1 : d;
}
void combat_begin(Combat *c) { *c = (Combat){.hp = kami.hp, .random = 42}; }
static int roll(Combat *c) {
  c->random = c->random * 1664525u + 1013904223u;
  return (int)((c->random >> 16) % 3) - 1;
}
void combat_turn(Combat *c, Player *p, int rage, bool herb, char *message, int size) {
  if (c->won || c->lost)
    return;
  int hit = 0;
  if (herb) {
    if (!player_heal(p)) {
      snprintf(message, (size_t)size, "Kein Heilkraut benutzt. Waehle neu.");
      return;
    }
  } else {
    hit = combat_damage(p->attack, kami.defense, roll(c));
    c->hp -= hit;
  }
  if (c->hp <= 0) {
    c->hp = 0;
    c->won = true;
    snprintf(message, (size_t)size,
             "Dein Hieb verursacht %d Schaden.\nDer Kami sinkt in sich zusammen.", hit);
    return;
  }
  int damage = combat_damage(kami.attack + rage, p->defense, roll(c));
  p->hp -= damage;
  if (p->hp <= 0) {
    p->hp = 0;
    c->lost = true;
    snprintf(message, (size_t)size, "Der Kami verursacht %d Schaden.\nDu faellst.",
             damage);
    return;
  }
  if (herb)
    snprintf(message, (size_t)size,
             "Das Kraut heilt bis zu 8 Lebenspunkte.\nDer Kami verursacht %d Schaden.",
             damage);
  else
    snprintf(message, (size_t)size,
             "Dein Hieb verursacht %d Schaden.\nDer Kami verursacht %d Schaden.", hit,
             damage);
}
