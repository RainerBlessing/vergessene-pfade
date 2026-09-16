#include "game.h"
#include <stdio.h>
#include <string.h>
bool game_init(Game *g, const char *assets) {
  memset(g, 0, sizeof *g);
  g->map = MAP_VILLAGE;
  g->x = 16;
  g->y = 21;
  g->dy = -1;
  g->npc = -1;
  g->player = (Player){.hp = 24, .max_hp = 24, .attack = 8, .defense = 2};
  char path[1024];
  snprintf(path, sizeof path, "%s/maps/village.map", assets);
  if (!map_load(&g->maps[MAP_VILLAGE], path))
    return false;
  snprintf(path, sizeof path, "%s/maps/forest.map", assets);
  return map_load(&g->maps[MAP_FOREST], path);
}
int game_npc_at(const Game *g, int x, int y) {
  for (int i = 0; i < NPC_COUNT; i++)
    if (npcs[i].map == g->map && npcs[i].x == x && npcs[i].y == y)
      return i;
  return -1;
}
bool game_knows(const Game *g, ObsId o) { return (g->obs & OBS(o)) != 0; }
int game_owned_items(const Game *g, ItemId *out) {
  int n = 0;
  for (int i = ITEM_NONE + 1; i < ITEM_COUNT; i++)
    if (g->player.inventory.quantities[i])
      out[n++] = (ItemId)i;
  return n;
}
int game_take_events(Game *g, GameEvent *out, int max) {
  int n = g->event_count < max ? g->event_count : max;
  memcpy(out, g->events, (size_t)n * sizeof *out);
  memmove(g->events, g->events + n, (size_t)(g->event_count - n) * sizeof *out);
  g->event_count -= n;
  return n;
}
static void emit(Game *g, EventType type, int a, int b) {
  if (g->event_count >= EVENT_LIMIT) {
    g->events_dropped++;
    return;
  }
  g->events[g->event_count++] = (GameEvent){type, g->map, g->x, g->y, a, b};
}
static bool matches(const Game *g, Obs needs, Obs forbids) {
  return (g->obs & needs) == needs && !(g->obs & forbids);
}
static void learn(Game *g, Obs grants, NoteId note) {
  for (int o = 0; o < OBS_COUNT; o++)
    if ((grants & OBS(o)) && !(g->obs & OBS(o))) {
      g->obs |= OBS(o);
      emit(g, EV_OBSERVE, o, 0);
    }
  if (note == NOTE_NONE)
    return;
  for (int i = 0; i < g->note_count; i++)
    if (g->notes[i] == note)
      return;
  if (g->note_count < NOTE_LIMIT)
    g->notes[g->note_count++] = note;
}
static void open_dialogue(Game *g, int npc, char examined, DialogueId dialogue) {
  g->npc = npc;
  g->examined = examined;
  g->dialogue = dialogue;
  g->page = 0;
  g->message[0] = 0;
  g->state = GAME_DIALOGUE;
}
static void talk(Game *g, int npc) {
  for (int i = 0; i < dialogue_rule_count; i++) {
    const DialogueRule *r = &dialogue_rules[i];
    if ((int)r->npc != npc || !matches(g, r->needs, r->forbids))
      continue;
    learn(g, r->grants, r->note);
    emit(g, EV_NPC_TALK, npc, r->dialogue);
    open_dialogue(g, npc, 0, r->dialogue);
    return;
  }
}
static const ExaminePoint *point_at(const Game *g, int x, int y) {
  char symbol = map_at(&g->maps[g->map], x, y);
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_ITEM || p->map != g->map || !matches(g, p->needs, 0))
      continue;
    if (p->kind == POINT_AT ? p->x == x && p->y == y : p->symbol == symbol)
      return p;
  }
  return NULL;
}
static const ExaminePoint *point_for_item(const Game *g, ItemId item) {
  for (int i = 0; i < examine_point_count; i++) {
    const ExaminePoint *p = &examine_points[i];
    if (p->kind == POINT_ITEM && p->item == item && matches(g, p->needs, 0))
      return p;
  }
  return NULL;
}
static void use_point(Game *g, const ExaminePoint *p, char examined) {
  if (p->gives != ITEM_NONE)
    inventory_add(&g->player.inventory, p->gives, 1);
  learn(g, p->grants, p->note);
  emit(g, EV_EXAMINE, examined, p->dialogue);
  open_dialogue(g, -1, examined, p->dialogue);
}
/* Repeated presses at the same target are counted, not logged again. */
static void examine_nothing(Game *g, int x, int y) {
  NothingTarget *last = &g->last_nothing;
  char symbol = map_at(&g->maps[g->map], x, y);
  const TileDef *tile = tile_def(symbol);
  snprintf(g->message, sizeof g->message, "%s: nichts Besonderes.",
           tile ? tile->name : "Dort");
  if (last->valid && last->map == g->map && last->x == x && last->y == y &&
      last->dx == g->dx && last->dy == g->dy) {
    last->repeat++;
    return;
  }
  emit(g, EV_EXAMINE_NOTHING, symbol, last->valid ? last->repeat : 0);
  *last = (NothingTarget){true, g->map, x, y, g->dx, g->dy, 0};
}
/* Facing tile first, then the tile underfoot (passable points cannot be faced). */
static void examine(Game *g) {
  int fx = g->x + g->dx, fy = g->y + g->dy;
  const ExaminePoint *p = point_at(g, fx, fy);
  if (p) {
    use_point(g, p, map_at(&g->maps[g->map], fx, fy));
    return;
  }
  p = point_at(g, g->x, g->y);
  if (p) {
    use_point(g, p, map_at(&g->maps[g->map], g->x, g->y));
    return;
  }
  examine_nothing(g, fx, fy);
}
static void inventory_action(Game *g, Action a) {
  ItemId owned[ITEM_COUNT];
  int count = game_owned_items(g, owned);
  if (a == ACT_CANCEL || a == ACT_INVENTORY) {
    g->state = GAME_EXPLORATION;
    return;
  }
  if (count == 0)
    return;
  if (a == ACT_UP)
    g->selection = (g->selection + count - 1) % count;
  if (a == ACT_DOWN)
    g->selection = (g->selection + 1) % count;
  if (a != ACT_CONFIRM)
    return;
  if (g->selection >= count)
    g->selection = 0;
  ItemId item = owned[g->selection];
  if (item == ITEM_HERB) {
    snprintf(g->message, sizeof g->message, "%s",
             player_heal(&g->player) ? "Das Kraut lindert deine Wunden."
                                     : "Du bist unverletzt.");
    return;
  }
  const ExaminePoint *p = point_for_item(g, item);
  if (p)
    use_point(g, p, 0);
}
static void notebook_action(Game *g, Action a) {
  int last = g->note_count > NOTES_PER_PAGE ? g->note_count - NOTES_PER_PAGE : 0;
  if (a == ACT_CANCEL || a == ACT_CONFIRM)
    g->state = GAME_EXPLORATION;
  else if (a == ACT_UP && g->scroll > 0)
    g->scroll--;
  else if (a == ACT_DOWN && g->scroll < last)
    g->scroll++;
}
static void move(Game *g, int dx, int dy) {
  g->dx = dx;
  g->dy = dy;
  if (game_npc_at(g, g->x + dx, g->y + dy) >= 0 ||
      !map_passable(&g->maps[g->map], g->x + dx, g->y + dy))
    return;
  g->x += dx;
  g->y += dy;
  if (!tile_def(map_at(&g->maps[g->map], g->x, g->y))->transition)
    return;
  for (int i = 0; i < TRANSITION_COUNT; i++) {
    const Transition *t = &transitions[i];
    if (g->map == t->map && g->x == t->x && g->y == t->y) {
      g->map = t->to_map;
      g->x = t->to_x;
      g->y = t->to_y;
      return;
    }
  }
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
  switch (g->state) {
  case GAME_NOTEBOOK:
    notebook_action(g, a);
    return;
  case GAME_DIALOGUE:
    if (a == ACT_CANCEL ||
        (a == ACT_CONFIRM && ++g->page >= dialogues[g->dialogue].count))
      g->state = GAME_EXPLORATION;
    return;
  case GAME_INVENTORY:
    inventory_action(g, a);
    return;
  case GAME_EXPLORATION:
    break;
  }
  if (a == ACT_CANCEL) {
    g->state = GAME_NOTEBOOK;
    g->scroll = g->note_count > NOTES_PER_PAGE ? g->note_count - NOTES_PER_PAGE : 0;
    emit(g, EV_NOTEBOOK_OPEN, g->note_count, 0);
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
    if (n >= 0)
      talk(g, n);
    else
      examine(g);
    return;
  }
  g->message[0] = 0;
  if (a == ACT_UP)
    move(g, 0, -1);
  else if (a == ACT_DOWN)
    move(g, 0, 1);
  else if (a == ACT_LEFT)
    move(g, -1, 0);
  else if (a == ACT_RIGHT)
    move(g, 1, 0);
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
