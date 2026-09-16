#include "quest.h"
const char *const quest_labels[4] = {
    "Suche Aoi im Haus im Nordwesten.", "Suche den Spiegel am Waldschrein.",
    "Bringe den Spiegel zurueck zu Aoi.", "Der Schrein leuchtet. Vielen Dank!"};
bool quest_start(QuestState *q) {
  if (*q != NOT_STARTED)
    return false;
  *q = ACTIVE;
  return true;
}
bool quest_find(QuestState *q, Player *p) {
  if (*q != ACTIVE || !inventory_add(&p->inventory, ITEM_LENS, 1))
    return false;
  *q = OBJECTIVE_FOUND;
  return true;
}
bool quest_complete(QuestState *q, Player *p) {
  if (*q != OBJECTIVE_FOUND || !inventory_remove(&p->inventory, ITEM_LENS, 1))
    return false;
  *q = COMPLETED;
  p->hp = p->max_hp;
  return true;
}
