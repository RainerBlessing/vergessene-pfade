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

/* Ein versperrter Schritt schweigt beim ersten Mal und antwortet beim zweiten
 * in dieselbe Richtung (#20). */
static void the_second_try_gets_an_answer(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 1, 1, -1, 0);
  game_action(&g, ACT_LEFT);
  assert(g.message[0] == 0); /* einmal dagegenlaufen sagt nichts */
  game_action(&g, ACT_LEFT);
  assert(strstr(g.message, "Dorfmauer") != 0);
  assert(strstr(g.message, "versperrt den Weg") != 0);
}

static void a_new_direction_starts_over(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 1, 1, -1, 0);
  game_action(&g, ACT_LEFT);
  game_action(&g, ACT_UP); /* andere Richtung, auch versperrt */
  assert(g.message[0] == 0);
  game_action(&g, ACT_UP);
  assert(g.message[0] != 0);
  /* Ein Schritt, der gelingt, setzt zurueck. */
  game_action(&g, ACT_DOWN);
  assert(g.y == 2 && g.message[0] == 0);
  face(&g, MAP_VILLAGE, 1, 1, -1, 0);
  game_action(&g, ACT_LEFT);
  assert(g.message[0] == 0);
}

static void the_stone_says_it_does_not_budge(void) {
  Game g;
  start(&g);
  face(&g, MAP_FOREST, STONE_START_X, (int8_t)(STONE_START_Y + 1), 0, -1);
  game_action(&g, ACT_UP);
  assert(g.message[0] == 0);
  game_action(&g, ACT_UP);
  assert(strstr(g.message, "ruehrt sich nicht") != 0);
  assert(g.stone_y == STONE_START_Y); /* und er liegt weiter, wo er lag */
}

static void a_person_in_the_way_says_so(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 5, 5, 0, -1); /* unter Sumi */
  game_action(&g, ACT_UP);
  assert(g.message[0] == 0);
  game_action(&g, ACT_UP);
  assert(strstr(g.message, "jemand im Weg") != 0);
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
  const Action look[] = {ACT_INVENTORY, ACT_CONFIRM}; /* nur die Scherben dabei */
  play(&g, look, 2);
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

/* Kintsugi and the compromise: the long way round, step by step. */
static void read_out(Game *g) {
  while (g->state == GAME_DIALOGUE)
    game_action(g, ACT_CONFIRM);
}
static void take_the_shards(Game *g) {
  face(g, MAP_FOREST, 38, 21, 0, -1); /* the offering stone */
  game_action(g, ACT_CONFIRM);
  read_out(g);
  const Action look[] = {ACT_INVENTORY, ACT_CONFIRM};
  play(g, look, 2); /* the mark burnt into the bottom */
  read_out(g);
  face(g, MAP_VILLAGE, 4, 7, 0, -1); /* the same mark beside Sumi's door */
  game_action(g, ACT_CONFIRM);
  read_out(g);
  face(g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(g, ACT_CONFIRM);
  assert(g->dialogue == D_SUMI_OWNER);
  read_out(g);
}
/* The piece that belongs into the gap that is open now. */
static void set_fitting_piece(Game *g) {
  uint8_t pieces[MEND_PIECES];
  uint8_t count = game_mend_pieces(g, pieces);
  for (uint8_t i = 0; i < count; i++)
    if (pieces[i] == g->mend_placed)
      g->selection = i;
  game_action(g, ACT_CONFIRM);
}

/* Wer die beiden Zeichen verbunden hat, bekommt von Oriha eine Antwort darauf
 * -- die Geschichte selbst erzaehlt weiter nur Sumi. */
static void oriha_answers_the_marks(void) {
  Game g;
  start(&g);
  g.bag[ITEM_SHARDS] = 1;
  g.obs |= OBS(OBS_BOWL_SHARDS);
  face(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_ORIHA_SHARDS); /* ohne Zeichen: die Frage nach der Geschichte */
  read_out(&g);
  g.obs |= OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK);
  face(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_ORIHA_MARK);
  read_out(&g);
  assert(!game_knows(&g, OBS_BOWL_OWNER)); /* sie verraet die Geschichte nicht */
  g.obs |= OBS(OBS_BOWL_OWNER);
  face(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_ORIHA_MEND);
}

static void mend_the_bowl(void) {
  Game g;
  start(&g);
  take_the_shards(&g);
  face(&g, MAP_VILLAGE, 24, 5, 0, -1); /* Oriha, once she knows the story */
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_ORIHA_MEND);
  read_out(&g);
  assert(g.state == GAME_MEND);
  /* A piece that does not fit costs nothing. */
  uint8_t pieces[MEND_PIECES];
  uint8_t count = game_mend_pieces(&g, pieces);
  for (uint8_t i = 0; i < count; i++)
    if (pieces[i] != g.mend_placed)
      g.selection = i;
  game_action(&g, ACT_CONFIRM);
  assert(g.mend_placed == 0 && g.state == GAME_MEND);
  for (int i = 0; i < MEND_PIECES; i++)
    set_fitting_piece(&g);
  assert(g.mend_placed == MEND_PIECES);
  assert(g.dialogue == D_MEND_DONE && game_knows(&g, OBS_BOWL_DRYING));
  read_out(&g);
  assert(game_tile(&g, MAP_VILLAGE, 22, 4) == 'b'); /* it stands on her shelf */
  /* The lacquer dries while the player is in the forest. */
  assert(!game_knows(&g, OBS_BOWL_READY));
  face(&g, MAP_VILLAGE, 16, 1, 0, -1);
  game_action(&g, ACT_UP);
  assert(g.map == MAP_FOREST && !game_knows(&g, OBS_BOWL_READY));
  face(&g, MAP_FOREST, 24, 38, 0, 1);
  game_action(&g, ACT_DOWN);
  assert(g.map == MAP_VILLAGE && game_knows(&g, OBS_BOWL_READY));
  assert(game_tile(&g, MAP_VILLAGE, 22, 4) == 'q');
  face(&g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_ORIHA_READY && g.bag[ITEM_BOWL] == 1);
}

/* Everything the foreman wants to hear before he stakes out a new boundary. */
static void prepare_the_compromise(Game *g) {
  take_the_shards(g);
  face(g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(g, ACT_CONFIRM);
  read_out(g);
  for (int i = 0; i < MEND_PIECES; i++)
    set_fitting_piece(g);
  read_out(g);
  g->obs |= OBS(OBS_BOWL_READY);
  face(g, MAP_VILLAGE, 24, 5, 0, -1);
  game_action(g, ACT_CONFIRM);
  read_out(g);
  assert(g->bag[ITEM_BOWL] == 1);
  /* The fox, the tracks and the ledger. */
  face(g, MAP_FOREST, 5, 21, 0, -1);
  game_action(g, ACT_CONFIRM);
  read_out(g);
  face(g, MAP_VILLAGE, 11, 11, 0, -1);
  game_action(g, ACT_CONFIRM);
  read_out(g);
  face(g, MAP_FOREST, 5, 21, 0, -1);
  const Action use[] = {ACT_INVENTORY, ACT_CONFIRM};
  play(g, use, 2);
  read_out(g);
  face(g, MAP_FOREST, 8, 20, 0, -1);
  game_action(g, ACT_CONFIRM);
  read_out(g);
  assert(game_knows(g, OBS_TRACKS));
  face(g, MAP_FOREST, 12, 31, 0, -1); /* the order book in the camp */
  game_action(g, ACT_CONFIRM);
  read_out(g);
  assert(game_knows(g, OBS_LEDGER_DEBT));
}

static void the_bowl_calms_the_spirit(void) {
  Game g;
  start(&g);
  prepare_the_compromise(&g);
  face(&g, MAP_FOREST, 25, 12, 0, -1);
  game_action(&g, ACT_UP);
  assert(g.state == GAME_ENCOUNTER);
  uint8_t options[ENCOUNTER_OPTION_LIMIT];
  uint8_t count = game_encounter_options(&g, options);
  for (uint8_t i = 0; i < count; i++)
    if (encounter_options[options[i]].action == ENC_OFFER)
      g.selection = i;
  game_action(&g, ACT_CONFIRM);
  assert(g.mood == MOOD_CALM && game_knows(&g, OBS_KAMI_CALMED));
  assert(g.dialogue == D_ENC_OFFER_BOWL);
}

static void stake_out_a_new_boundary(void) {
  Game g;
  start(&g);
  prepare_the_compromise(&g);
  g.obs |= OBS(OBS_KAMI_CALMED);
  face(&g, MAP_FOREST, 11, 32, 0, -1); /* Daigo stands at 11,31 */
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_DAIGO_OFFER && game_knows(&g, OBS_DAIGO_DEAL));
  read_out(&g);
  assert(g.daigo_follows);
  /* He walks in the player's footsteps, so he is there at every stake. */
  for (int i = 0; i < STAKE_COUNT; i++) {
    g.x = (int8_t)stakes[i].x;
    g.y = (int8_t)(stakes[i].y + 1);
    g.daigo_x = g.x;
    g.daigo_y = (int8_t)(g.y + 1);
    g.dx = 0;
    g.dy = -1;
    game_action(&g, ACT_UP); /* onto the stake spot, Daigo follows */
    assert(g.y == (int8_t)stakes[i].y && g.daigo_y == (int8_t)(stakes[i].y + 1));
    game_action(&g, ACT_CONFIRM);
    assert((g.staked & (1u << i)) != 0);
    read_out(&g);
  }
  assert(g.outcome == OUT_MEND && !g.daigo_follows);
  assert(game_tile(&g, MAP_FOREST, stakes[0].x, stakes[0].y) == 'p');
  assert(game_tile(&g, MAP_FOREST, 13, 30) == 'z'); /* deadwood for the village */
  assert(game_tile(&g, MAP_FOREST, 22, 10) == 'x'); /* the old edge stays cut */
  face(&g, MAP_VILLAGE, 5, 5, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_SUMI_MEND);
  read_out(&g);
  face(&g, MAP_FOREST, 11, 32, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(g.dialogue == D_DAIGO_MEND);
}

/* Untersuchen: der Blick zaehlt zuerst, aber was daneben liegt, wird gefunden. */
static void examine_reaches_all_four_neighbours(void) {
  Game g;
  start(&g);
  /* Das Hauszeichen liegt noerdlich, die Blickrichtung zeigt nach Westen. */
  face(&g, MAP_VILLAGE, 4, 7, -1, 0);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_HOUSE_MARK));
  assert(g.dialogue == D_X_HOUSE_MARK);
}

static void the_facing_tile_wins(void) {
  Game g;
  start(&g);
  /* Mulde im Norden, Schleifspur im Sueden: der Blick entscheidet. */
  face(&g, MAP_FOREST, 24, 13, 0, -1);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_STONE_HOLLOW) && !game_knows(&g, OBS_STONE_DRAGGED));
  while (g.state == GAME_DIALOGUE)
    game_action(&g, ACT_CONFIRM);
  face(&g, MAP_FOREST, 24, 13, 0, 1);
  game_action(&g, ACT_CONFIRM);
  assert(game_knows(&g, OBS_STONE_DRAGGED));
}

static void nothing_around_still_says_so(void) {
  Game g;
  start(&g);
  face(&g, MAP_FOREST, 24, 25, 0, -1); /* freies Feld, nichts in Reichweite */
  game_action(&g, ACT_CONFIRM);
  assert(g.state == GAME_EXPLORATION);
  assert(strstr(g.message, "nichts Besonderes") != 0);
}

/* Das Heilkraut heilt -- und tut sonst nichts. */
static void healing_leaves_the_world_alone(void) {
  Game g;
  start(&g);
  see_stone_and_hollow(&g);
  face(&g, MAP_FOREST, 25, 16, -1, 0);
  game_action(&g, ACT_LEFT); /* Stein einmal zur Seite geschoben */
  assert(g.stone_x == 23);
  g.bag[ITEM_HERB] = 1;
  g.hp = 10;
  g.daigo_follows = true;
  g.daigo_x = 20;
  g.daigo_y = 20;
  const Action use[] = {ACT_INVENTORY, ACT_CONFIRM};
  play(&g, use, 2);
  assert(g.hp > 10);
  assert(g.stone_x == 23 && g.stone_y == STONE_START_Y);
  assert(g.daigo_x == 20 && g.daigo_y == 20);
}

/* Das Kraut wirkt auch, wenn der Fuchs nicht genau in Blickrichtung liegt. */
static void an_item_reaches_the_neighbour(void) {
  Game g;
  start(&g);
  face(&g, MAP_FOREST, 5, 21, 0, -1);
  game_action(&g, ACT_CONFIRM); /* verletzter Fuchs */
  read_out(&g);
  g.bag[ITEM_HERB] = 1;
  face(&g, MAP_FOREST, 6, 20, 1, 0); /* neben dem Bau, Blick nach Osten */
  const Action use[] = {ACT_INVENTORY, ACT_CONFIRM};
  play(&g, use, 2);
  assert(game_knows(&g, OBS_FOX_TENDED) && g.bag[ITEM_HERB] == 0);
}

/* Die Tasche haelt niemanden fest (#19). */
static void a_step_closes_the_bag(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 16, 5, 0, 1);
  g.bag[ITEM_HERB] = 1; /* nur ein Gegenstand: nichts zu waehlen */
  game_action(&g, ACT_INVENTORY);
  assert(g.state == GAME_INVENTORY);
  game_action(&g, ACT_DOWN);
  assert(g.state == GAME_EXPLORATION && g.y == 6);
}

static void two_items_still_get_chosen(void) {
  Game g;
  start(&g);
  face(&g, MAP_VILLAGE, 16, 5, 0, 1);
  g.bag[ITEM_HERB] = 1;
  g.bag[ITEM_SHARDS] = 1;
  game_action(&g, ACT_INVENTORY);
  game_action(&g, ACT_DOWN);
  assert(g.state == GAME_INVENTORY && g.selection == 1 && g.y == 5);
  /* Links und rechts waehlen nichts aus -- sie gehen. */
  game_action(&g, ACT_LEFT);
  assert(g.state == GAME_EXPLORATION && g.x == 15);
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

/* Die Begegnungstafel zeigt nur eine Seite; mehrseitige Texte waeren dort
 * nicht zu lesen. Die Szenen nach Sieg und Niederlage sind ausgenommen -- die
 * laufen als Dialog ab. */
static void encounter_texts_are_single_page(void) {
  for (int a = 0; a < ENC_COUNT; a++)
    for (int m = 0; m < MOOD_COUNT; m++) {
      uint8_t d = encounter_lines[a][m];
      assert(d == D_NONE || dialogues[d].count == 1);
    }
  for (int i = 0; i < encounter_offer_count; i++)
    assert(dialogues[encounter_offers[i].dialogue].count == 1);
  assert(dialogues[D_ENC_APPEAR].count == 1);
}

/* Die Karten kommen aus assets/maps und werden auch anderswo bearbeitet.
 * Dieser Test haelt fest, was der Slice braucht: dass man ueberall hinkommt
 * und nirgends in einer Ecke mit nur einem Ausgang landet (#18). */
#define FOREST_CELLS (64 * 64)
static void the_forest_stays_walkable(void) {
  Game g;
  start(&g);
  g.map = MAP_FOREST;
  static bool seen[FOREST_CELLS];
  static uint16_t queue[FOREST_CELLS];
  uint16_t head = 0, tail = 0;
  uint8_t w = map_width(MAP_FOREST), h = map_height(MAP_FOREST);
  queue[tail++] = (uint16_t)(38 * w + 24); /* das Nordtor des Dorfes */
  seen[38 * w + 24] = true;
  while (head < tail) {
    uint16_t at = queue[head++];
    int8_t x = (int8_t)(at % w), y = (int8_t)(at / w);
    const int8_t step[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (uint8_t i = 0; i < 4; i++) {
      int8_t nx = (int8_t)(x + step[i][0]), ny = (int8_t)(y + step[i][1]);
      if (nx < 0 || ny < 0 || (uint8_t)nx >= w || (uint8_t)ny >= h)
        continue;
      uint16_t n = (uint16_t)(ny * w + nx);
      if (!seen[n] && game_passable(&g, MAP_FOREST, nx, ny)) {
        seen[n] = true;
        queue[tail++] = n;
      }
    }
  }
  /* Alles, was der Slice im Wald braucht. */
  const uint8_t must[][2] = {{5, 21}, {12, 31}, {11, 32}, {38, 21}, {23, 16},
                             {24, 13}, {24, 12}, {14, 14}, {22, 15}, {30, 14},
                             {8, 19}, {34, 13}, {24, 11}};
  for (uint8_t i = 0; i < sizeof must / sizeof must[0]; i++)
    assert(seen[must[i][1] * w + must[i][0]]);
  /* Sackgassen: Felder mit nur einem Ausgang. Das alte Gitter hatte 44. */
  uint8_t dead = 0;
  for (uint8_t y = 0; y < h; y++)
    for (uint8_t x = 0; x < w; x++) {
      if (!game_passable(&g, MAP_FOREST, (int8_t)x, (int8_t)y))
        continue;
      uint8_t outs = 0;
      const int8_t step[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
      for (uint8_t i = 0; i < 4; i++)
        if (game_passable(&g, MAP_FOREST, (int8_t)(x + step[i][0]),
                          (int8_t)(y + step[i][1])))
          outs++;
      if (outs <= 1)
        dead++;
    }
  assert(dead <= 20);
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
  the_second_try_gets_an_answer();
  a_new_direction_starts_over();
  the_stone_says_it_does_not_budge();
  a_person_in_the_way_says_so();
  fox_unlocks_tracks();
  bowl_story_needs_both_marks();
  retreat_ends_encounter();
  waiting_calms_the_spirit();
  fight_changes_world();
  offering_shards_angers_it();
  pushing_needs_both_observations();
  restore_old_boundary();
  a_stuck_stone_rolls_back();
  oriha_answers_the_marks();
  mend_the_bowl();
  the_bowl_calms_the_spirit();
  stake_out_a_new_boundary();
  examine_reaches_all_four_neighbours();
  the_facing_tile_wins();
  nothing_around_still_says_so();
  healing_leaves_the_world_alone();
  an_item_reaches_the_neighbour();
  a_step_closes_the_bag();
  two_items_still_get_chosen();
  notes_are_unique();
  encounter_texts_are_single_page();
  the_forest_stays_walkable();
  every_dialogue_fits_the_screen();
  printf("alle Tests bestanden\n");
  return 0;
}
