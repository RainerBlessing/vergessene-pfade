#include "inventory.h"
const ItemDef items[ITEM_COUNT] = {{"Beifusstee", 8}, {"Bernsteinspiegel", 0}};
bool inventory_add(Inventory *i, ItemId item, int n) {
  if (item < 0 || item >= ITEM_COUNT || n <= 0 || n > 99 - i->quantities[item])
    return false;
  i->quantities[item] += (uint8_t)n;
  return true;
}
bool inventory_remove(Inventory *i, ItemId item, int n) {
  if (item < 0 || item >= ITEM_COUNT || n <= 0 || n > i->quantities[item])
    return false;
  i->quantities[item] -= (uint8_t)n;
  return true;
}
bool player_heal(Player *p) {
  if (p->hp >= p->max_hp || !inventory_remove(&p->inventory, ITEM_HERB, 1))
    return false;
  p->hp += items[ITEM_HERB].heal;
  if (p->hp > p->max_hp)
    p->hp = p->max_hp;
  return true;
}
