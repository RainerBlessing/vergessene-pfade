#include "content.h"
const char *const obs_names[OBS_COUNT] = {
    "ASKED_BY_SUMI", "KENTA_STARE",  "CLAW_MARKS_EDGE",
    "FRESH_STUMPS",  "BROKEN_ROPE",  "SHRINE_INSCRIPTION",
    "STONE_DRAGGED", "STONE_HOLLOW", "BOWL_SHARDS",
    "BOWL_MARK",     "HOUSE_MARK",   "BOWL_OWNER",
    "LEDGER_DEBT",   "FOX_WOUNDED",  "MIO_HERB",
    "FOX_TENDED",    "TRACKS",       "KAMI_SEEN",
    "KAMI_ANGERED",  "STONE_MOVED",  "BOWL_DRYING",
    "BOWL_READY",    "KAMI_CALMED",  "DAIGO_DEAL",
    "GREY_TRACE",    "MORNING",      "TEASED"};
const char *const outcome_names[OUTCOME_COUNT] = {"NONE", "FIGHT", "BOUNDARY", "MEND"};
const char *const phase_names[PHASE_COUNT] = {"BEFORE", "MORNING"};

const Npc npcs[NPC_COUNT] = {{MAP_VILLAGE, 5, 4, 0, "SUMI", "Sumi / Dorfaelteste"},
                             {MAP_VILLAGE, 24, 4, 1, "ORIHA", "Oriha / Lackmeisterin"},
                             {MAP_VILLAGE, 11, 10, 2, "MIO", "Mio"},
                             {MAP_VILLAGE, 18, 2, 3, "KENTA", "Kenta / Holzfaeller"},
                             {MAP_FOREST, 11, 31, 4, "DAIGO", "Daigo / Vorarbeiter"}};

const Transition transitions[TRANSITION_COUNT] = {
    {MAP_VILLAGE, 16, 0, MAP_FOREST, 24, 38}, {MAP_FOREST, 24, 39, MAP_VILLAGE, 16, 1}};

/* Pages: at most 3 lines of at most 36 characters (checked by test_game). */
const Dialogue dialogues[DIALOGUE_COUNT] = {
#include "../shared/dialogues.inc"
    [D_ORIHA_OWNER] = {1,
                       {"Sumis Grossmutter also. Dann hat\ndiese Schale eine lange "
                        "Geschichte.\nLass mich darueber nachdenken."}},
    /* An empty page shows the last message: the numbers of the closing round. */
    [D_ENC_OFFER_BOWL] = {2,
                          {"Du stellst die geflickte Schale\nins Moos. Die goldenen "
                           "Naehte\nfangen das Licht.",
                           "Der Kami beugt sich darueber.\nDas Knurren hoert auf. "
                           "Es setzt\nsich neben die Schale."}},
    [D_SCENE_TEASER] = {2,
                        {"Du sitzt lange am Schrein.\nDer Wald ist ruhig. Die graue\n"
                         "Stelle bleibt grau.",
                         "Irgendwo hinter den Bergen liegen\nweitere Doerfer. Auch "
                         "dort, denkst\ndu, wird etwas leiser geworden sein."}},
};

/* At most 2 lines of 36 characters; no digits or slashes (test_game). */
const char *const notes[NOTE_COUNT] = {
#include "../shared/notes.inc"
};

/* Wohin die Antwort auf einen Ausgang fuehrt: in die Nacht (PC) oder zur Schlusstafel
 * (C64). */
#define OPEN_AFTER_OUTCOME OPEN_NIGHT
#define MARKS (OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK))
const DialogueRule dialogue_rules[] = {
#include "../shared/dialogue_rules_1.inc"
#include "../shared/dialogue_rules_2_morning.inc"
#include "../shared/dialogue_rules_3.inc"
};
const int dialogue_rule_count = (int)(sizeof dialogue_rules / sizeof dialogue_rules[0]);

const ExaminePoint examine_points[] = {
#include "../shared/examine_points_1.inc"
#include "../shared/examine_points_2_late.inc"
#include "../shared/examine_points_3.inc"
#include "../shared/examine_points_4_morning.inc"
#include "../shared/examine_points_futon.inc"
};
const int examine_point_count = (int)(sizeof examine_points / sizeof examine_points[0]);

#include "../shared/tile_macros.inc"
#define TILE(map, x, y, sym, needs, out, ph, tag) {map, x, y, sym, needs, out, ph, tag}
const TileOverride tile_overrides[] = {
#include "../shared/tile_overrides.inc"
};
/* What each outcome changes, at once and again the next morning. Every outcome
 * shows a gain and a loss in both phases; a test checks that. */
const TileOverride outcome_changes[] = {
#include "../shared/tile_changes.inc"
};
const int outcome_change_count =
    (int)(sizeof outcome_changes / sizeof outcome_changes[0]);
const int tile_override_count = (int)(sizeof tile_overrides / sizeof tile_overrides[0]);

const char *const mood_names[MOOD_COUNT] = {"ZORNIG", "MISSTRAUISCH", "RUHIG"};
const char *const encounter_action_names[ENC_COUNT] = {[ENC_WAIT] = "WAIT",
                                                       [ENC_OFFER] = "OFFER",
                                                       [ENC_ATTACK] = "ATTACK",
                                                       [ENC_HEAL] = "HEAL",
                                                       [ENC_RETREAT] = "RETREAT"};
const Mood encounter_transitions[ENC_COUNT][MOOD_COUNT] = {
    /* from:        ANGRY       WARY        CALM */
    [ENC_WAIT] = {MOOD_WARY, MOOD_WARY, MOOD_CALM},
    [ENC_OFFER] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM}, /* offers override this */
    [ENC_ATTACK] = {MOOD_ANGRY, MOOD_ANGRY, MOOD_ANGRY},
    [ENC_HEAL] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM},
    [ENC_RETREAT] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM},
};
const DialogueId encounter_lines[ENC_COUNT][MOOD_COUNT] = {
    [ENC_WAIT] = {D_ENC_WAIT_ANGRY, D_ENC_WAIT_WARY, D_ENC_WAIT_CALM},
    [ENC_OFFER] = {D_NONE, D_NONE, D_NONE},
    [ENC_ATTACK] = {D_ENC_ATTACK, D_ENC_ATTACK, D_ENC_ATTACK},
    [ENC_HEAL] = {D_NONE, D_NONE, D_NONE},
    [ENC_RETREAT] = {D_ENC_RETREAT, D_ENC_RETREAT, D_ENC_RETREAT},
};
const EncounterOption encounter_options[] = {
    {ENC_WAIT, "STEHEN BLEIBEN", OPT_PEACE},   {ENC_OFFER, "DARBRINGEN", OPT_PEACE},
    {ENC_ATTACK, "ANGREIFEN", OPT_BOTH},       {ENC_HEAL, "HEILKRAUT NEHMEN", OPT_FIGHT},
    {ENC_RETREAT, "ZURUECKWEICHEN", OPT_BOTH},
};
const int encounter_option_count =
    (int)(sizeof encounter_options / sizeof encounter_options[0]);
_Static_assert(sizeof encounter_options / sizeof encounter_options[0] <=
                   ENCOUNTER_OPTION_LIMIT,
               "raise ENCOUNTER_OPTION_LIMIT");
const EncounterOffer encounter_offers[] = {
    /* The mended bowl comes first: it is what the player would hold out. */
    {ITEM_BOWL, OBS(OBS_KAMI_CALMED), MOOD_CALM, D_ENC_OFFER_BOWL, N_KAMI_CALM,
     ITEM_BOWL},
    {ITEM_SHARDS, OBS(OBS_KAMI_ANGERED), MOOD_ANGRY, D_ENC_OFFER_SHARDS, N_KAMI_SHARDS,
     ITEM_NONE},
};
const int encounter_offer_count =
    (int)(sizeof encounter_offers / sizeof encounter_offers[0]);

const MendPiece mend_pieces[MEND_PIECES] = {
    {"Bodenstueck", "Der Boden fehlt."},
    {"Wandstueck", "Die Wand hat einen Sprung."},
    {"Randstueck", "Am Rand fehlt ein Stueck."},
    {"Stueck mit dem Zeichen", "Neben dem Zeichen klafft ein Riss."},
};
const int mend_display[MEND_PIECES] = {2, 0, 3, 1};

/* The stakes stand on the animal tracks, between the old stones. */
const StakeSpot stakes[STAKE_COUNT] = {{14, 14}, {22, 15}, {30, 14}};

const char *const night_choices[NIGHT_CHOICES] = NIGHT_CHOICE_TEXTS;

/* Buildings say what they are, whether or not anyone is in. */
const Place places[] = {
#include "../shared/places.inc"
};
const int place_count = (int)(sizeof places / sizeof places[0]);

/* One page, skippable. Blank entries are spacing. */
const char *const title_page[TITLE_LINES] = {
    "DIE VERGESSENEN PFADE",
    "",
    "Erkunde das Dorf und den Wald.",
    "Sprich mit den Menschen, untersuche",
    "Auffaelliges und verbinde deine",
    "Beobachtungen. Es gibt mehrere Wege,",
    "den Konflikt zu loesen.",
    "",
    "PFEILE / WASD  Bewegen",
    "ENTER          Reden, untersuchen",
    "I              Tasche, Gegenstaende",
    "ESC            Notizbuch",
    "",
    "Stell dich vor ein Objekt und sieh",
    "es an. Das Notizbuch haelt Funde fest.",
};
