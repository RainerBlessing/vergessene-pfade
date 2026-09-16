#include "game.h"
#include <stdio.h>
#define CHECK(x)                                                                         \
  do {                                                                                   \
    if (!(x)) {                                                                          \
      fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x);                               \
      return 1;                                                                          \
    }                                                                                    \
  } while (0)
int main(int argc, char **argv) {
  Game g;
  CHECK(argc == 2);
  CHECK(game_init(&g, argv[1]));
  CHECK(!map_passable(&g.maps[0], -1, 0));
  CHECK(!map_passable(&g.maps[0], 0, 0));
  CHECK(map_passable(&g.maps[0], 10, 26));
  game_action(&g, ACT_RIGHT);
  CHECK(g.x == 11);
  g.x = 1;
  g.y = 1;
  game_action(&g, ACT_LEFT);
  CHECK(g.x == 1);
  int x, y;
  game_camera(&g, &x, &y);
  CHECK(x == 0 && y == 0);
  g.map = 0;
  g.x = 10;
  g.y = 9;
  game_action(&g, ACT_UP);
  CHECK(g.map == 1 && g.y == 27);
  game_action(&g, ACT_DOWN);
  CHECK(g.map == 0 && g.y == 9);
  g.map = 1;
  g.x = 10;
  g.y = 8;
  game_action(&g, ACT_UP);
  CHECK(g.y == 8);
  CHECK(game_npc_at(&g, 10, 7) == 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.npc == 0 && g.page == 0);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.page == 1);
  game_action(&g, ACT_CANCEL);
  CHECK(g.state == GAME_EXPLORATION);
  Inventory inv = {0};
  CHECK(inventory_add(&inv, ITEM_HERB, 99));
  CHECK(!inventory_add(&inv, ITEM_HERB, 1));
  CHECK(inventory_remove(&inv, ITEM_HERB, 99));
  CHECK(!inventory_remove(&inv, ITEM_HERB, 1));
  CHECK(!inventory_add(&inv, ITEM_COUNT, 1));
  CHECK(g.player.inventory.quantities[ITEM_HERB] == 2);
  g.player.hp = 23;
  CHECK(player_heal(&g.player) && g.player.hp == 24);
  CHECK(!player_heal(&g.player));
  g.map = 1;
  g.x = 22;
  g.y = 10;
  g.dx = 0;
  g.dy = -1;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_DIALOGUE && g.npc == 1);
  for (int i = 0; i < 4 && g.state == GAME_DIALOGUE; i++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION);
  game_action(&g, ACT_INVENTORY);
  CHECK(g.state == GAME_INVENTORY);
  game_action(&g, ACT_CANCEL);
  QuestState q = NOT_STARTED;
  Player p = {.max_hp = 24};
  CHECK(!quest_complete(&q, &p));
  CHECK(quest_start(&q));
  CHECK(!quest_start(&q));
  CHECK(quest_find(&q, &p));
  CHECK(p.inventory.quantities[ITEM_LENS] == 1);
  CHECK(!quest_find(&q, &p));
  CHECK(quest_complete(&q, &p));
  CHECK(q == COMPLETED && p.hp == 24 && p.inventory.quantities[ITEM_LENS] == 0);
  CHECK(!quest_complete(&q, &p));
  CHECK(g.quest == ACTIVE);
  CHECK(quest_find(&g.quest, &g.player));
  g.map = 1;
  g.x = 10;
  g.y = 8;
  g.dy = -1;
  g.dx = 0;
  game_action(&g, ACT_CONFIRM);
  CHECK(g.quest == COMPLETED && g.dialogue == 5);
  game_action(&g, ACT_CANCEL);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.dialogue == 6);
  for (int a = 0; a < 12; a++)
    for (int d = 0; d < 12; d++)
      for (int r = -1; r <= 1; r++) {
        int hit = combat_damage(a, d, r);
        CHECK(hit >= 1 && hit < 14);
      }
  g.state = GAME_EXPLORATION;
  g.quest = ACTIVE;
  g.player.hp = 24;
  g.map = 0;
  g.x = 38;
  g.y = 9;
  game_action(&g, ACT_UP);
  CHECK(g.state == GAME_COMBAT);
  for (int turn = 0; turn < 20 && !g.combat.won && !g.combat.lost; turn++)
    game_action(&g, ACT_CONFIRM);
  CHECK(g.combat.won && g.quest == OBJECTIVE_FOUND);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.state == GAME_EXPLORATION);
  g.quest = ACTIVE;
  g.x = 38;
  g.y = 9;
  g.player.hp = 1;
  game_action(&g, ACT_UP);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.combat.lost);
  game_action(&g, ACT_CONFIRM);
  CHECK(g.map == 1 && g.player.hp == 24 && g.quest == ACTIVE);
  for (int d = 0; d < 7; d++)
    for (int page = 0; page < dialogues[d].count; page++) {
      int lines = 1, columns = 0;
      for (const char *c = dialogues[d].pages[page]; *c; c++) {
        if (*c == '\n') {
          CHECK(columns <= 36);
          columns = 0;
          lines++;
        } else
          columns++;
      }
      CHECK(columns <= 36 && lines <= 3);
    }
  puts("Town, NPCs, Map, collision, movement and camera pass.");
  return 0;
}
