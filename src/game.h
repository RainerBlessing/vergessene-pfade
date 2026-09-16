#ifndef GAME_H
#define GAME_H
#include "combat.h"
#include "content.h"
#include "inventory.h"
#include "quest.h"
#include "world.h"
typedef enum {
  ACT_NONE,
  ACT_UP,
  ACT_DOWN,
  ACT_LEFT,
  ACT_RIGHT,
  ACT_CONFIRM,
  ACT_CANCEL,
  ACT_INVENTORY,
  ACT_DEBUG,
  ACT_COLLISION
} Action;
typedef enum {
  GAME_EXPLORATION,
  GAME_DIALOGUE,
  GAME_INVENTORY,
  GAME_COMBAT,
  GAME_PAUSED
} GameState;
typedef struct {
  Map maps[MAP_COUNT];
  int map, x, y, dx, dy;
  GameState state;
  int npc, page, selection, dialogue;
  QuestState quest;
  Player player;
  Combat combat;
  char message[128];
  bool debug, collision;
} Game;
bool game_init(Game *g, const char *assets);
void game_action(Game *g, Action a);
int game_npc_at(const Game *g, int x, int y);
void game_camera(const Game *g, int *x, int *y);
#endif
