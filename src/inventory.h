#ifndef INVENTORY_H
#define INVENTORY_H
#include <stdbool.h>
#include <stdint.h>
typedef enum { ITEM_HERB, ITEM_LENS, ITEM_SWORD, ITEM_COUNT } ItemId;
typedef struct {
  const char *name;
  int price, heal;
} ItemDef;
extern const ItemDef items[ITEM_COUNT];
extern const ItemId shop_stock[2];
typedef struct {
  uint8_t quantities[ITEM_COUNT];
} Inventory;
typedef struct {
  int hp, max_hp, attack, defense, gold;
  Inventory inventory;
} Player;
bool inventory_add(Inventory *i, ItemId item, int quantity);
bool inventory_remove(Inventory *i, ItemId item, int quantity);
bool shop_buy(Player *p, ItemId item);
bool player_heal(Player *p);
#endif
