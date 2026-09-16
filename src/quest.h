#ifndef QUEST_H
#define QUEST_H
#include "inventory.h"
typedef enum { NOT_STARTED, ACTIVE, OBJECTIVE_FOUND, COMPLETED } QuestState;
extern const char *const quest_labels[4];
bool quest_start(QuestState *q);
bool quest_find(QuestState *q, Player *p);
bool quest_complete(QuestState *q, Player *p);
#endif
