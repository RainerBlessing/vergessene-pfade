/* Host tests for the C64 POC's rules. The game logic has no C64 in it, so it
 * can be played through here before it ever runs on the machine. */
#include "../src/game.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void play(Game *g, const Action *actions, unsigned n) {
  for (unsigned i = 0; i < n; i++)
    game_action(g, actions[i]);
}
static void start(Game *g) {
  game_init(g);
  game_action(g, ACT_CONFIRM); /* leave the title page */
  while (g->state == GAME_DIALOGUE)
    game_action(g, ACT_CONFIRM); /* read the arrival scene */
  assert(g->state == GAME_EXPLORATION);
}
static void face(Game *g, uint8_t map, int8_t x, int8_t y, int8_t dx, int8_t dy) {
  g->map = map;
  g->x = x;
  g->y = y;
  g->dx = dx;
  g->dy = dy;
}

static void walk_between_maps(void) {
  Game g;
  start(&g);
  assert(g.map == MAP_FOREST);
  face(&g, MAP_FOREST, 24, 38, 0, 1);
  game_action(&g, ACT_DOWN);
  assert(g.map == MAP_VILLAGE && g.x == 16 && g.y == 1);
  face(&g, MAP_VILLAGE, 16, 1, 0, -1);
  game_action(&g, ACT_UP);
  assert(g.map == MAP_FOREST && g.x == 24 && g.y == 38);
}

static void walls_block(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 1, 1, -1, 0);
  game_action(&g, ACT_LEFT);
  assert(g.x == 1 && g.y == 1); /* the village wall stays put */
}

static void fox_unlocks_tracks(void) {
  Game g;
  start(&g);
  /* The fox's den is at 5,20; the fox is examined from the tile below it. */
  face(&g, MAP_FOREST, 5, 21, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_FOX_WOUNDED));
  game_action(&g, ACT_CONFIRM); /* close the text */
  /* Mio's herb; she gives it once the fox is known to be hurt. */
  face(&g, MAP_VILLAGE, 11, 11, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_MIO_HERB && g.bag[ITEM_HERB] == 1);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  /* Tracks are invisible until the fox has been tended. */
  face(&g, MAP_FOREST, 8, 20, 0, -1);
  assert(game_tile(&g, MAP_FOREST, 8, 19) != 't');
  face(&g, MAP_FOREST, 5, 21, 0, -1);
  const Action use[] = {ACT_INVENTORY, ACT_CONFIRM};
  play(&g, use, 2);
  assert(game_knows(&g, OBS_FOX_TENDED) && g.bag[ITEM_HERB] == 0);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  assert(game_tile(&g, MAP_FOREST, 8, 19) == 't');
  face(&g, MAP_FOREST, 8, 20, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_TRACKS));
}

static void bowl_story_needs_both_marks(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_TASK); /* without the marks she gives the task */
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  /* The shards lie on the offering stone at 38,20, looked at from below. */
  face(&g, MAP_FOREST, 38, 21, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.bag[ITEM_SHARDS] == 1 && game_knows(&g, OBS_BOWL_SHARDS));
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  const Action look[] = {ACT_INVENTORY, ACT_DOWN, ACT_CONFIRM};
  play(&g, look, 3);
  assert(game_knows(&g, OBS_BOWL_MARK));
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  /* The house mark is next to Sumi's door. */
  face(&g, MAP_VILLAGE, 4, 7, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_HOUSE_MARK));
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  face(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_OWNER && game_knows(&g, OBS_BOWL_OWNER));
}

/* Stepping onto the guarded moss calls the spirit up. */
static void step_into_the_grove(Game *g) {
  start(g);
  face(g, MAP_FOREST, 24, 12, 0, -1);
  assert(game_tile(g, MAP_FOREST, 24, 11) == 'h');
  game_action(g, ACT_UP);
  assert(g->state == GAME_ENCOUNTER && game_knows(g, OBS_KAMI_SEEN));
}

static void retreat_ends_encounter(void) {
  Game g;
  step_into_the_grove(&g);
  game_action(&g, ACT_CANCEL); /* highlights retreating */
  game_action(&g, ACT_CONFIRM);
  assert(g.state == GAME_EXPLORATION && g.outcome == OUT_NONE);
  assert(g.y == 13); /* pushed one step back out of the grove */
}

static void waiting_calms_the_spirit(void) {
  Game g;
  step_into_the_grove(&g);
  assert(g.mood == MOOD_ANGRY);
  game_action(&g, ACT_CONFIRM); /* the first option is standing still */
  assert(g.mood == MOOD_WARY);
  game_action(&g, ACT_CONFIRM);
  assert(g.mood == MOOD_WARY); /* waiting alone never makes it calm */
}

static void fight_changes_world(void) {
  Game g;
  step_into_the_grove(&g);
  assert(game_tile(&g, MAP_VILLAGE, 23, 13) != 'W');
  for (int i = 0; i < 20 && g.outcome == OUT_NONE; i++) {
    uint8_t options[ENCOUNTER_OPTION_LIMIT];
    uint8_t count = game_encounter_options(&g, options);
    for (uint8_t k = 0; k < count; k++)
      if (encounter_options[options[k]].action == ENC_ATTACK)
        g.selection = k;
    game_action(&g, ACT_CONFIRM);
  }
  assert(g.outcome == OUT_FIGHT);
  assert(g.dialogue == D_ENC_VICTORY);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  assert(game_tile(&g, MAP_VILLAGE, 23, 13) == 'W');  /* wood for the winter */
  assert(game_tile(&g, MAP_FOREST, 20, 6) == 'x');    /* the grove is felled */
  face(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_FOUGHT);
}

/* The old boundary: the stone goes back into its hollow, one push per step.
 * Only someone who has seen both the drag marks and the hollow can push it. */
static void see_stone_and_hollow(Game *g) {
  face(g, MAP_FOREST, 24, 17, 0, -1); /* the stone, from the south */
  game_action(g, ACT_CONFIRM);
  assert(game_knows(g, OBS_STONE_DRAGGED));
  while (g->state == GAME_DIALOGUE)
    game_action(g, ACT_CONFIRM);
  face(g, MAP_FOREST, 24, 13, 0, -1); /* the hollow it was dragged out of */
  game_action(g, ACT_CONFIRM);
  assert(game_knows(g, OBS_STONE_HOLLOW));
  while (g->state == GAME_DIALOGUE)
    game_action(g, ACT_CONFIRM);
}

static void pushing_needs_both_observations(void) {
  Game g;
  start(&g);
  assert(!game_can_push(&g));
  face(&g, MAP_FOREST, 24, 17, 0, -1);
  game_action(&g, ACT_UP);
  assert(g.stone_y == STONE_START_Y && g.y == 17); /* nothing gives way */
}

static void restore_old_boundary(void) {
  Game g;
  start(&g);
  see_stone_and_hollow(&g);
  assert(game_can_push(&g));
  face(&g, MAP_FOREST, 24, 17, 0, -1);
  for (int i = 0; i < 4; i++)
    game_action(&g, ACT_UP);
  assert(g.stone_x == STONE_HOLLOW_X && g.stone_y == STONE_HOLLOW_Y);
  assert(g.outcome == OUT_BOUNDARY && g.dialogue == D_SCENE_BOUNDARY);
  assert(g.mood == MOOD_CALM);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  assert(game_tile(&g, MAP_FOREST, 16, 11) == 'h');  /* the grove edge is kept */
  assert(game_tile(&g, MAP_FOREST, 14, 28) == '.');  /* the camp loses ground */
  /* A grove at peace lets the player walk in -- beside the stone, which now
   * fills its hollow again. */
  face(&g, MAP_FOREST, 25, 12, 0, -1);
  game_action(&g, ACT_UP);
  assert(g.state == GAME_EXPLORATION && g.y == 11);
  face(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_BOUNDARY);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  face(&g, MAP_FOREST, 11, 32, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_DAIGO_BOUNDARY);
}

static void a_stuck_stone_rolls_back(void) {
  Game g;
  start(&g);
  see_stone_and_hollow(&g);
  face(&g, MAP_FOREST, 25, 16, -1, 0); /* shoved aside instead of homewards */
  game_action(&g, ACT_LEFT);
  assert(g.stone_x == 23 && game_knows(&g, OBS_STONE_MOVED));
  face(&g, MAP_FOREST, 23, 17, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_X_STONE_STUCK);
  assert(g.stone_x == STONE_START_X && g.stone_y == STONE_START_Y);
  assert(!game_knows(&g, OBS_STONE_MOVED));
}

static void offering_shards_angers_it(void) {
  Game g;
  step_into_the_grove(&g);
  g.bag[ITEM_SHARDS] = 1;
  game_action(&g, ACT_CONFIRM); /* stand still: wary */
  assert(g.mood == MOOD_WARY);
  uint8_t options[ENCOUNTER_OPTION_LIMIT];
  uint8_t count = game_encounter_options(&g, options);
  for (uint8_t k = 0; k < count; k++)
    if (encounter_options[options[k]].action == ENC_OFFER)
      g.selection = k;
  game_action(&g, ACT_CONFIRM);
  assert(g.mood == MOOD_ANGRY && game_knows(&g, OBS_KAMI_ANGERED));
}

static void notes_are_unique(void) {
  Game g;
  start(&g);
  face(&g, MAP_FOREST, 5, 21, 0, -1);
  game_action(&g, ACT_CONFIRM);
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  uint8_t before = g.note_count;
  game_action(&g, ACT_CONFIRM); /* looking again writes nothing new */
  assert(g.note_count == before);
}

static void every_dialogue_fits_the_screen(void) {
  for (int d = 1; d < DIALOGUE_COUNT; d++)
    for (int p = 0; p < dialogues[d].count; p++) {
      const char *text = dialogues[d].pages[p];
      assert(text);
      int line = 0, column = 0;
      for (const char *c = text; *c; c++) {
        if (*c == '\n') {
          line++;
          column = 0;
          continue;
        }
        column++;
        assert(column <= 38);
      }
      assert(line < 3);
    }
  for (int n = 1; n < NOTE_COUNT; n++) {
    assert(notes[n]);
    int column = 0;
    for (const char *c = notes[n]; *c; c++)
      if (*c == '\n')
        column = 0;
      else
        assert(++column <= 38);
  }
}

int main(void) {
  walk_between_maps();
  walls_block();
  fox_unlocks_tracks();
  bowl_story_needs_both_marks();
  retreat_ends_encounter();
  waiting_calms_the_spirit();
  fight_changes_world();
  offering_shards_angers_it();
  pushing_needs_both_observations();
  restore_old_boundary();
  a_stuck_stone_rolls_back();
  notes_are_unique();
  every_dialogue_fits_the_screen();
  printf("alle Tests bestanden\n");
  return 0;
}
