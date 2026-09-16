#ifndef CONTENT_H
#define CONTENT_H
typedef enum { ROLE_KEEPER, ROLE_MERCHANT, ROLE_RESIDENT, ROLE_SCOUT } NpcRole;
typedef struct {
  int map, x, y, sprite;
  NpcRole role;
  const char *name;
} Npc;
extern const Npc npcs[4];
typedef struct {
  int map, x, y, to_map, to_x, to_y;
} Transition;
extern const Transition transitions[2];
typedef struct {
  int count;
  const char *pages[3];
} Dialogue;
extern const Dialogue dialogues[7];
#endif
