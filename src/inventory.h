#ifndef INVENTORY_H
#define INVENTORY_H
#include <stdbool.h>
#include <stdint.h>
typedef enum { ITEM_NONE, ITEM_HERB, ITEM_SHARDS, ITEM_COUNT } ItemId;
typedef struct {
  const char *name;
  int heal;
} ItemDef;
extern const ItemDef items[ITEM_COUNT];
typedef struct {
  uint8_t quantities[ITEM_COUNT];
} Inventory;
typedef struct {
  int hp, max_hp, attack, defense;
  Inventory inventory;
} Player;
bool inventory_add(Inventory *i, ItemId item, int quantity);
bool inventory_remove(Inventory *i, ItemId item, int quantity);
bool player_heal(Player *p);
#endif
