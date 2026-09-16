#include "game.h"
#include <stdio.h>
#include <string.h>
bool game_init(Game *g, const char *assets) {
  memset(g, 0, sizeof *g);
  g->x = 10;
  g->y = 26;
  g->dy = -1;
  g->player = (Player){.hp = 24, .max_hp = 24, .attack = 6, .defense = 2, .gold = 18};
  inventory_add(&g->player.inventory, ITEM_HERB, 1);
  char path[1024];
  snprintf(path, sizeof path, "%s/maps/world.map", assets);
  if (!map_load(&g->maps[0], path))
    return false;
  snprintf(path, sizeof path, "%s/maps/town.map", assets);
  return map_load(&g->maps[1], path);
}
int game_npc_at(const Game *g, int x, int y) {
  for (int i = 0; i < 4; i++)
    if (npcs[i].map == g->map && npcs[i].x == x && npcs[i].y == y)
      return i;
  return -1;
}
void game_action(Game *g, Action a) {
  if (a == ACT_DEBUG) {
    g->debug = !g->debug;
    return;
  }
  if (a == ACT_COLLISION) {
    g->collision = !g->collision;
    return;
  }
  if (g->state == GAME_PAUSED) {
    if (a == ACT_CANCEL || a == ACT_CONFIRM)
      g->state = GAME_EXPLORATION;
    return;
  }
  if (g->state == GAME_EXPLORATION && a == ACT_CANCEL) {
    g->state = GAME_PAUSED;
    return;
  }
  if (g->state == GAME_COMBAT) {
    if (g->combat.won || g->combat.lost) {
      if (a == ACT_CONFIRM) {
        if (g->combat.lost) {
          g->map = 1;
          g->x = 15;
          g->y = 26;
          g->player.hp = g->player.max_hp;
          inventory_add(&g->player.inventory, ITEM_HERB, 1);
        }
        g->state = GAME_EXPLORATION;
        g->message[0] = 0;
      }
      return;
    }
    if (a == ACT_UP || a == ACT_DOWN)
      g->selection = 1 - g->selection;
    if (a == ACT_CONFIRM) {
      combat_turn(&g->combat, &g->player, g->selection == 1, g->message,
                  sizeof g->message);
      if (g->combat.won)
        quest_find(&g->quest, &g->player);
    }
    return;
  }
  if (g->state == GAME_DIALOGUE) {
    if (a == ACT_CANCEL ||
        (a == ACT_CONFIRM && ++g->page >= dialogues[g->dialogue].count))
      g->state = npcs[g->npc].role == ROLE_MERCHANT && a == ACT_CONFIRM
                     ? GAME_SHOP
                     : GAME_EXPLORATION;
    return;
  }
  if (g->state == GAME_INVENTORY || g->state == GAME_SHOP) {
    int count = g->state == GAME_SHOP ? 2 : ITEM_COUNT;
    if (a == ACT_CANCEL || a == ACT_INVENTORY) {
      g->state = GAME_EXPLORATION;
      return;
    }
    if (a == ACT_UP)
      g->selection = (g->selection + count - 1) % count;
    if (a == ACT_DOWN)
      g->selection = (g->selection + 1) % count;
    if (a == ACT_CONFIRM) {
      if (g->state == GAME_SHOP)
        snprintf(g->message, sizeof g->message, "%s",
                 shop_buy(&g->player, shop_stock[g->selection])
                     ? "Gekauft. Eine gute Reise!"
                     : "Kauf nicht moeglich: Gold / Besitz.");
      else if (g->selection == ITEM_HERB)
        snprintf(g->message, sizeof g->message, "%s",
                 player_heal(&g->player) ? "Der Tee gibt dir neue Lebenskraft."
                                         : "Volle Lebenskraft oder kein Tee.");
      else
        snprintf(g->message, sizeof g->message, "%s",
                 g->selection == ITEM_SWORD ? "Reiseklinge: angelegt, +2 Angriff."
                                            : "Der Schreinspiegel. Bring ihn Aoi.");
    }
    return;
  }
  if (a == ACT_INVENTORY) {
    g->state = GAME_INVENTORY;
    g->selection = 0;
    g->message[0] = 0;
    return;
  }
  if (a == ACT_CONFIRM) {
    int n = game_npc_at(g, g->x + g->dx, g->y + g->dy);
    if (n >= 0) {
      g->npc = n;
      g->dialogue = n;
      if (npcs[n].role == ROLE_KEEPER) {
        if (g->quest == NOT_STARTED)
          quest_start(&g->quest);
        else if (g->quest == ACTIVE)
          g->dialogue = 4;
        else if (g->quest == OBJECTIVE_FOUND) {
          quest_complete(&g->quest, &g->player);
          g->dialogue = 5;
        } else
          g->dialogue = 6;
      }
      g->selection = 0;
      g->message[0] = 0;
      g->page = 0;
      g->state = GAME_DIALOGUE;
    }
    return;
  }
  g->message[0] = 0;
  int dx = 0, dy = 0;
  if (a == ACT_UP)
    dy = -1;
  if (a == ACT_DOWN)
    dy = 1;
  if (a == ACT_LEFT)
    dx = -1;
  if (a == ACT_RIGHT)
    dx = 1;
  if (dx || dy) {
    g->dx = dx;
    g->dy = dy;
    if (game_npc_at(g, g->x + dx, g->y + dy) < 0 &&
        map_passable(&g->maps[g->map], g->x + dx, g->y + dy)) {
      g->x += dx;
      g->y += dy;
      if (g->map == beast.map && g->x == beast.x && g->y == beast.y &&
          g->quest == ACTIVE) {
        combat_begin(&g->combat);
        g->state = GAME_COMBAT;
        g->selection = 0;
        snprintf(
            g->message, sizeof g->message,
            "Ein Dornengeist bewacht den Spiegel.\nANGREIFEN oder BEIFUSSTEE trinken.");
      }
      if (tile_def(map_at(&g->maps[g->map], g->x, g->y))->transition) {
        for (int i = 0; i < 2; i++) {
          const Transition *t = &transitions[i];
          if (g->map == t->map && g->x == t->x && g->y == t->y) {
            g->map = t->to_map;
            g->x = t->to_x;
            g->y = t->to_y;
            break;
          }
        }
      }
    }
  }
}
void game_camera(const Game *g, int *x, int *y) {
  *x = g->x - 10;
  *y = g->y - 5;
  int mx = g->maps[g->map].width - 20, my = g->maps[g->map].height - 10;
  if (*x > mx)
    *x = mx;
  if (*y > my)
    *y = my;
  if (*x < 0)
    *x = 0;
  if (*y < 0)
    *y = 0;
}
