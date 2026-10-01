#ifndef SHARED_TALK_H
#define SHARED_TALK_H
#include <stddef.h>
#include <stdint.h>
/* Rules that the PC and the C64 version play identically. Included after
 * content.h, which supplies DialogueRule, dialogue_rules and the Obs bits. */

/* First rule that speaks for `npc` now, or NULL. `carried` is indexed by ItemId:
 * a rule that would hand over something the player still carries is skipped, so
 * the next rule speaks. */
static inline const DialogueRule *talk_select(int npc, Obs obs, int outcome, int phase,
                                              const uint8_t *carried) {
  for (int i = 0; i < dialogue_rule_count; i++) {
    const DialogueRule *r = &dialogue_rules[i];
    if ((int)r->npc != npc || (obs & r->needs) != r->needs || (obs & r->forbids))
      continue;
    if ((int)r->outcome != OUT_ANY && (int)r->outcome != outcome)
      continue;
    if ((int)r->phase != PHASE_ANY && (int)r->phase != phase)
      continue;
    if (r->gives != ITEM_NONE && carried[r->gives])
      continue;
    return r;
  }
  return NULL;
}
#endif
