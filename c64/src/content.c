#include "content.h"

const ItemDef items[ITEM_COUNT] = {
    {"", 0}, {"Heilkraut", 8}, {"Scherben", 0}, {"Geflickte Schale", 0}};

const Npc npcs[NPC_COUNT] = {{MAP_VILLAGE, 5, 4, 'S', "Sumi / Dorfaelteste"},
                             {MAP_VILLAGE, 24, 4, 'O', "Oriha / Lackmeisterin"},
                             {MAP_VILLAGE, 11, 10, 'M', "Mio"},
                             {MAP_VILLAGE, 18, 2, 'K', "Kenta / Holzfaeller"},
                             {MAP_FOREST, 11, 31, 'D', "Daigo / Vorarbeiter"}};

const Transition transitions[TRANSITION_COUNT] = {
    {MAP_VILLAGE, 16, 0, MAP_FOREST, 24, 38}, {MAP_FOREST, 24, 39, MAP_VILLAGE, 16, 1}};

/* Pages: at most 3 lines of at most 38 characters. */
const Dialogue dialogues[DIALOGUE_COUNT] = {
#include "../../shared/dialogues.inc"
    [D_NONE] = {0, {0}},
    /* Stein und Mulde verweisen aufeinander, sobald beide bekannt sind (#14). */
    /* An empty page shows the last message: the numbers of the closing round. */
    /* Eine Seite: die Begegnungstafel blaettert nicht, dort zaehlt jede Zeile. */
    [D_ENC_OFFER_BOWL] = {1,
                          {"Du stellst die geflickte Schale ins\nMoos. Der Kami "
                           "beugt sich darueber.\nEs setzt sich neben die Schale."}},
};

const char *const notes[NOTE_COUNT] = {
#include "../../shared/notes.inc"
};

/* Wohin die Antwort auf einen Ausgang fuehrt: in die Nacht (PC) oder zur Schlusstafel (C64). */
#define OPEN_AFTER_OUTCOME OPEN_END
#define MARKS (OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK))
const DialogueRule dialogue_rules[] = {
#include "../../shared/dialogue_rules_a.inc"
#include "../../shared/dialogue_rules_b.inc"
};
const uint8_t dialogue_rule_count =
    (uint8_t)(sizeof dialogue_rules / sizeof dialogue_rules[0]);

const ExaminePoint examine_points[] = {
    /* Village */
    {POINT_AT, MAP_VILLAGE, 4, 6, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE, OBS(OBS_BOWL_MARK),
     OBS(OBS_HOUSE_MARK), D_X_HOUSE_MARK_MATCH, N_MARKS_MATCH, OUT_NONE},
    {POINT_AT, MAP_VILLAGE, 4, 6, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_HOUSE_MARK), D_X_HOUSE_MARK, N_HOUSE_MARK, OUT_NONE},
    /* Der Stapel sagt, was man sieht: nach jedem Ausgang etwas anderes. */
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'W', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WOOD_FIGHT, NOTE_NONE, OUT_FIGHT},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'W', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WOOD_BOUNDARY, NOTE_NONE, OUT_BOUNDARY},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'W', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WOOD_MEND, NOTE_NONE, OUT_MEND},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'W', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WOOD, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'n', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_INN_SIGN, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'w', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WORKSHOP_SIGN, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_VILLAGE, 0, 0, 'c', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_WORKBENCH, NOTE_NONE, OUT_NONE},
    /* Forest: the moved stone comes before the general rule for stones. A stone
     * stuck somewhere between both ends is rolled back to the drag marks. */
    {POINT_STONE, MAP_FOREST, 0, 0, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_STONE_MOVED), 0, D_X_STONE_STUCK, NOTE_NONE, OUT_NONE},
    {POINT_STONE, MAP_FOREST, 0, 0, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_STONE_HOLLOW), OBS(OBS_STONE_DRAGGED), D_X_MOVED_STONE_MATCH, N_STONE_MATCH,
     OUT_NONE},
    {POINT_STONE, MAP_FOREST, 0, 0, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_STONE_DRAGGED), D_X_MOVED_STONE, N_DRAGGED, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'd', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_STONE_DRAGGED), D_X_DRAGGED, N_DRAGGED, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'G', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_CLAW_MARKS_EDGE), D_X_CLAW, N_CLAW, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'm', ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_STONE_DRAGGED), OBS(OBS_STONE_HOLLOW), D_X_HOLLOW_MATCH, N_STONE_MATCH,
     OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'm', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_STONE_HOLLOW), D_X_HOLLOW, N_HOLLOW, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'x', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_FRESH_STUMPS), D_X_STUMPS, N_STUMPS, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'Y', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_BROKEN_ROPE), D_X_ROPE, N_ROPE, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'O', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_SHRINE_INSCRIPTION), D_X_INSCRIPTION, N_INSCRIPTION, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'o', ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_BOWL_SHARDS), 0, D_X_ALTAR_EMPTY, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'o', ITEM_NONE, ITEM_SHARDS, ITEM_NONE, 0,
     OBS(OBS_BOWL_SHARDS), D_X_SHARDS, N_SHARDS, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'k', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_LEDGER_DEBT), D_X_LEDGER, N_LEDGER, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'A', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0, D_X_TENT,
     NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'F', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_FOX_WOUNDED), D_X_FOX, N_FOX, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'f', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_FOX_TENDED, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'P', ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_FOX_TENDED), 0, D_X_TRACKS, NOTE_NONE, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 't', ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_FOX_TENDED), OBS(OBS_TRACKS), D_X_TRACKS, N_TRACKS, OUT_NONE},
    /* Items */
    {POINT_ITEM, MAP_FOREST, 0, 0, 'F', ITEM_HERB, ITEM_NONE, ITEM_HERB,
     OBS(OBS_FOX_WOUNDED), OBS(OBS_FOX_TENDED), D_I_TEND_FOX, N_FOX_TENDED, OUT_NONE},
    {POINT_ITEM, 0, 0, 0, 0, ITEM_SHARDS, ITEM_NONE, ITEM_NONE, OBS(OBS_HOUSE_MARK),
     OBS(OBS_BOWL_MARK), D_I_BOWL_MARK_MATCH, N_MARKS_MATCH, OUT_NONE},
    {POINT_ITEM, 0, 0, 0, 0, ITEM_SHARDS, ITEM_NONE, ITEM_NONE, 0, OBS(OBS_BOWL_MARK),
     D_I_BOWL_MARK, N_BOWL_MARK, OUT_NONE},
};
const uint8_t examine_point_count =
    (uint8_t)(sizeof examine_points / sizeof examine_points[0]);

/* Sobald der Handel steht, sind die drei Stellen zu sehen: Suchen gehoert nicht
 * zu dieser Entscheidung, sie ist laengst gefallen (#21). */
#define MARK(x, y) {MAP_FOREST, x, y, 'P', OBS(OBS_DAIGO_DEAL), OUT_ANY}
#define TRACK(x, y) {MAP_FOREST, x, y, 't', OBS(OBS_FOX_TENDED), OUT_ANY}
const TileOverride tile_overrides[] = {
    MARK(14, 14),
    MARK(22, 15),
    MARK(30, 14),
    /* What the fight changed comes first: it is the newer state of the world. */
    {MAP_VILLAGE, 23, 13, 'W', 0, OUT_FIGHT},
    {MAP_VILLAGE, 24, 13, 'W', 0, OUT_FIGHT},
    {MAP_FOREST, 20, 6, 'x', 0, OUT_FIGHT},
    {MAP_FOREST, 24, 7, 'x', 0, OUT_FIGHT},
    /* Alte Grenze: the grove keeps its edge, the camp loses its ground. */
    {MAP_FOREST, 16, 11, 'h', 0, OUT_BOUNDARY},
    {MAP_FOREST, 14, 28, '.', 0, OUT_BOUNDARY},
    /* Kompromiss: deadwood for the village, the old edge stays cut. */
    {MAP_FOREST, 13, 30, 'z', 0, OUT_MEND},
    {MAP_FOREST, 22, 10, 'x', 0, OUT_MEND},
    /* Orihas shelf: the bowl rests there while the lacquer dries. */
    {MAP_VILLAGE, 22, 4, 'q', OBS(OBS_BOWL_READY), OUT_ANY},
    {MAP_VILLAGE, 22, 4, 'b', OBS(OBS_BOWL_DRYING), OUT_ANY},
    {MAP_FOREST, 5, 20, 'f', OBS(OBS_FOX_TENDED), OUT_ANY},
    /* Paw prints: around the camp, along the grove edge, to the stone gap. */
    TRACK(8, 19),
    TRACK(9, 18),
    TRACK(10, 17),
    TRACK(11, 16),
    TRACK(12, 15),
    TRACK(14, 14),
    TRACK(16, 14),
    TRACK(18, 15),
    TRACK(20, 15),
    TRACK(22, 15),
    TRACK(26, 15),
    TRACK(28, 15),
    TRACK(30, 14),
    TRACK(32, 14),
    TRACK(34, 13),
};
const uint8_t tile_override_count =
    (uint8_t)(sizeof tile_overrides / sizeof tile_overrides[0]);

const char *const mood_names[MOOD_COUNT] = {"ZORNIG", "MISSTRAUISCH", "RUHIG"};
const uint8_t encounter_transitions[ENC_COUNT][MOOD_COUNT] = {
    /* from:        ANGRY       WARY        CALM */
    [ENC_WAIT] = {MOOD_WARY, MOOD_WARY, MOOD_CALM},
    [ENC_OFFER] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM}, /* offers override this */
    [ENC_ATTACK] = {MOOD_ANGRY, MOOD_ANGRY, MOOD_ANGRY},
    [ENC_HEAL] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM},
    [ENC_RETREAT] = {MOOD_ANGRY, MOOD_WARY, MOOD_CALM},
};
const uint8_t encounter_lines[ENC_COUNT][MOOD_COUNT] = {
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
const uint8_t encounter_option_count =
    (uint8_t)(sizeof encounter_options / sizeof encounter_options[0]);
const EncounterOffer encounter_offers[] = {
    {ITEM_BOWL, OBS(OBS_KAMI_CALMED), MOOD_CALM, D_ENC_OFFER_BOWL, N_KAMI_CALM,
     ITEM_BOWL},
    {ITEM_SHARDS, OBS(OBS_KAMI_ANGERED), MOOD_ANGRY, D_ENC_OFFER_SHARDS, N_KAMI_SHARDS,
     ITEM_NONE},
};
const uint8_t encounter_offer_count =
    (uint8_t)(sizeof encounter_offers / sizeof encounter_offers[0]);

const MendPiece mend_pieces[MEND_PIECES] = {
    {"Bodenstueck", "Der Boden fehlt."},
    {"Wandstueck", "Die Wand hat einen Sprung."},
    {"Randstueck", "Am Rand fehlt ein Stueck."},
    {"Stueck mit dem Zeichen", "Neben dem Zeichen klafft ein Riss."},
};
const uint8_t mend_display[MEND_PIECES] = {2, 0, 3, 1};

/* The stakes stand on the animal tracks, between the old stones. */
const StakeSpot stakes[STAKE_COUNT] = {{14, 14}, {22, 15}, {30, 14}};

/* Buildings say what they are, whether or not anyone is in. */
const Place places[] = {
#include "../../shared/places.inc"
};
const uint8_t place_count = (uint8_t)(sizeof places / sizeof places[0]);

/* Was das Dorf am Ende hat -- und was es dafuer nicht mehr hat. */
const char *const closing_line[OUTCOME_COUNT] = {
    [OUT_NONE] = "",
    [OUT_FIGHT] = "Stille im Hain. Das Dorf hat Holz.",
    [OUT_BOUNDARY] = "Die Steine stehen. Das Lager nicht.",
    [OUT_MEND] = "Drei Pfaehle und eine Schale im Moos.",
};

/* Die Schlusstafel. Der Satz zum Ausgang steht darueber. */
const char *const closing_page[CLOSING_LINES] = {
    "HIER ENDET DER AUSSCHNITT",
    "",
    "In der vollen Fassung folgt eine",
    "Nacht im Gasthaus und ein Morgen, der",
    "zeigt, was aus der Entscheidung",
    "geworden ist. Am Schrein liegt dann",
    "eine Spur, die niemand erklaert.",
    "",
    "RETURN: weiterlaufen",
};

/* One page, skippable. Blank entries are spacing. */
const char *const title_page[TITLE_LINES] = {
    "DIE VERGESSENEN PFADE",
    "C64-POC: Erkunden, Verstehen, Handeln",
    "",
    "Erkunde das Dorf und den Wald.",
    "Sprich mit den Menschen, untersuche",
    "Auffaelliges und verbinde deine",
    "Beobachtungen.",
    "",
    "W A S D       Bewegen",
    "RETURN        Reden, untersuchen",
    "I             Tasche, Gegenstaende",
    "N             Notizbuch",
    "",
    "Stell dich vor ein Objekt und sieh",
    "es an. RETURN startet.",
};
