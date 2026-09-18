#include "content.h"
const char *const obs_names[OBS_COUNT] = {
    "ASKED_BY_SUMI", "KENTA_STARE",        "CLAW_MARKS_EDGE", "FRESH_STUMPS",
    "BROKEN_ROPE",   "SHRINE_INSCRIPTION", "STONE_DRAGGED",   "STONE_HOLLOW",
    "BOWL_SHARDS",   "BOWL_MARK",          "HOUSE_MARK",      "BOWL_OWNER",
    "LEDGER_DEBT",   "FOX_WOUNDED",        "MIO_HERB",        "FOX_TENDED",
    "TRACKS",        "KAMI_SEEN",          "KAMI_ANGERED",    "STONE_MOVED"};
const char *const outcome_names[OUTCOME_COUNT] = {"NONE", "FIGHT", "BOUNDARY", "MEND"};

const Npc npcs[NPC_COUNT] = {{MAP_VILLAGE, 5, 4, 0, "SUMI", "Sumi / Dorfaelteste"},
                             {MAP_VILLAGE, 24, 4, 1, "ORIHA", "Oriha / Lackmeisterin"},
                             {MAP_VILLAGE, 11, 10, 2, "MIO", "Mio"},
                             {MAP_VILLAGE, 18, 2, 3, "KENTA", "Kenta / Holzfaeller"},
                             {MAP_FOREST, 11, 31, 4, "DAIGO", "Daigo / Vorarbeiter"}};

const Transition transitions[TRANSITION_COUNT] = {
    {MAP_VILLAGE, 16, 0, MAP_FOREST, 24, 38}, {MAP_FOREST, 24, 39, MAP_VILLAGE, 16, 1}};

/* Pages: at most 3 lines of at most 36 characters (checked by test_game). */
const Dialogue dialogues[DIALOGUE_COUNT] = {
    [D_SCENE_ARRIVAL] = {3,
                         {"Die Abkuerzung durch den Wald.\nSeit Jahren bist du "
                          "sie nicht\nmehr gegangen.",
                          "Ein Schrei. Kenta rennt an dir\nvorbei und verschwindet "
                          "nach\nSueden, Richtung Dorf.",
                          "Vom alten Hain her faehrt ein\nWindstoss auf dich zu und "
                          "wirft\ndich zurueck. Dann ist es still."}},
    [D_SUMI_TASK] = {2,
                     {"Du bist zurueck. Gut. Wir brauchen\njede Hand. Im Wald greift "
                      "ein\nGeist unsere Holzfaeller an.",
                      "Ohne Holz frieren wir im Winter.\nGeh in den Wald und "
                      "vertreibe\nden Geist. Bitte."}},
    [D_SUMI_WAITING] = {1, {"Der Wald liegt hinter dem Nordtor.\nSei vorsichtig."}},
    [D_SUMI_OWNER] = {2,
                      {"Dieses Zeichen... das ist unseres.\nDie Schale gehoerte "
                       "meiner\nGrossmutter.",
                       "Sie hat den Waldschrein gepflegt.\nNach ihrem Tod ging "
                       "niemand\nmehr hin. Ich auch nicht."}},
    [D_SUMI_OWNER_KNOWN] = {1,
                            {"Grossmutter sagte immer, der Wald\nvergesse nichts. "
                             "Ich hielt das\nfuer einen Spruch."}},
    [D_ORIHA] = {2,
                 {"Ich bin Oriha. Ich flicke, was\nzerbrochen ist. Mit Lack und\nGold.",
                  "Aber ich flicke nichts, dessen\nGeschichte ich nicht kenne."}},
    [D_ORIHA_SHARDS] = {1,
                        {"Scherben einer Opferschale.\nWem gehoerte sie? Ohne "
                         "ihre\nGeschichte fasse ich sie nicht an."}},
    [D_ORIHA_OWNER] = {1,
                       {"Sumis Grossmutter also. Dann hat\ndiese Schale eine lange "
                        "Geschichte.\nLass mich darueber nachdenken."}},
    [D_MIO] = {2,
               {"Im Wald wohnt ein Fuchs. Er hat\nmir mal zugesehen, wie ich\nBeeren "
                "gesammelt habe.",
                "Seit die Baeume fallen, sehe ich\nihn nicht mehr. Ob er Angst hat?"}},
    [D_MIO_HERB] = {1,
                    {"Der Fuchs ist verletzt? Oh nein.\nHier, mein Heilkraut. "
                     "Leg es ihm\nauf die Wunde. Bitte!"}},
    [D_MIO_HERB_MORE] = {1,
                         {"Du hast es aufgebraucht? Hier,\nmein letztes. Der Fuchs "
                          "braucht\nes noetiger als wir."}},
    [D_MIO_HERB_AGAIN] = {1,
                          {"Hast du ihm schon geholfen? Nimm\ndas Kraut aus der "
                           "Tasche, wenn\ndu bei ihm bist."}},
    [D_MIO_THANKS] = {1,
                      {"Du hast ihm geholfen? Danke!\nFuechse merken sich so "
                       "etwas."}},
    [D_KENTA] = {2,
                 {"Ich geh da nicht mehr rein! Der\nalte Baum hat mich angeschaut.\nMit "
                  "... mit Rinde!",
                  "Daigo sagt, ich soll mich nicht\nanstellen. Aber ihn hat es "
                  "auch\nerwischt. Ha."}},
    [D_KENTA_AGAIN] = {1, {"Nein. Nicht mal fuer Reiskuchen.\nDer Baum GUCKT."}},
    [D_DAIGO] = {2,
                 {"Noch ein Neugieriger? Der Geist\nhat drei meiner Leute "
                  "verletzt.\nMich eingeschlossen.",
                  "Ich brauche das Holz. Frag nicht\nwarum. Sorg einfach dafuer,\ndass "
                  "wir arbeiten koennen."}},
    [D_DAIGO_LEDGER] = {1,
                        {"Du hast in mein Buch gesehen?\nJa. Liefere ich nicht, "
                         "verliert\ndas Lager alles."}},
    [D_X_WOOD] = {1, {"Kaum Holz auf dem Stapel.\nUnd der Winter kommt bald."}},
    [D_X_HOUSE_MARK] = {1, {"Neben Sumis Tuer ist ein\nHauszeichen eingebrannt."}},
    [D_X_HOUSE_MARK_MATCH] = {1,
                              {"Neben Sumis Tuer ist ein\nHauszeichen eingebrannt. "
                               "Dasselbe\nwie auf der Schale?"}},
    [D_X_CLAW] = {1,
                  {"Tiefe Kratzspuren im Stein.\nSie enden hier. Dahinter "
                   "keine\neinzige."}},
    [D_X_DRAGGED] = {1,
                     {"Die Erde ist aufgewuehlt. Eine\nSchleifspur fuehrt nach "
                      "Norden.\nJemand hat den Stein bewegt."}},
    [D_X_HOLLOW] = {1, {"Eine Mulde mit Moosrand. Hier lag\nlange etwas Schweres."}},
    [D_X_STUMPS] = {1, {"Frisches Harz. Diese Baeume\nfielen vor wenigen Tagen."}},
    [D_X_ROPE] =
        {1, {"Um den alten Baum hing ein Seil.\nEs ist gerissen, nicht\nverrottet."}},
    [D_X_INSCRIPTION] = {1,
                         {"Ein kleiner, verwitterter Schrein.\nEingeritzt: Bis zu "
                          "den drei\nSteinen und nicht weiter."}},
    [D_X_ALTAR_EMPTY] = {1, {"Der Opferstein ist leer."}},
    [D_X_SHARDS] = {1,
                    {"Scherben einer Schale. In einer\nliegen vertrocknete "
                     "Beeren.\nDu nimmst sie behutsam mit."}},
    [D_X_LEDGER] = {1,
                    {"Ein Auftragsbuch. Holz fuer die\nStadt, Frist in zehn "
                     "Tagen.\nLetzte Seite: Schulden."}},
    [D_X_TENT] = {1, {"Zelte der Holzfaeller. Es riecht\nnach Harz und Rauch."}},
    [D_X_FOX] = {1,
                 {"Ein Fuchs vor seinem Bau. Er ist\nverletzt und knurrt leise. "
                  "An\nder Pfote klebt dunkler Lehm."}},
    [D_X_FOX_TENDED] = {1, {"Der Fuchs schlaeft vor dem Bau.\nDer Verband haelt."}},
    [D_X_TRACKS] = {1,
                    {"Tierspuren. Sie machen einen\nweiten Bogen um das Lager "
                     "und\nfolgen dem Rand des Hains."}},
    [D_I_TEND_FOX] = {2,
                      {"Du legst Mios Kraut auf die\nWunde. Der Fuchs haelt "
                       "still.\nDann humpelt er in den Bau.",
                       "Als du dich aufrichtest, faellt\ndir etwas am Boden auf: "
                       "Faehrten.\nUeberall Faehrten."}},
    [D_ENC_APPEAR] = {1,
                      {"Zwischen den Staemmen richtet sich\netwas auf. Ein Geweih "
                       "aus Aesten.\nEin Gesicht wie Rinde."}},
    [D_ENC_WAIT_ANGRY] = {1,
                          {"Du bleibst stehen und ruehrst dich\nnicht. Das Knurren "
                           "wird leiser."}},
    [D_ENC_WAIT_WARY] = {1,
                         {"Du wartest. Es sieht dich an und\nweicht nicht. Aeste "
                          "knacken."}},
    [D_ENC_WAIT_CALM] = {1, {"Ihr steht euch ruhig gegenueber.\nDer Wald atmet."}},
    [D_ENC_ATTACK] = {1, {"Du machst einen Schritt nach vorn.\nDer Hain wird laut."}},
    [D_ENC_RETREAT] = {1,
                       {"Du weichst zurueck. Der Wind legt\nsich, die Aeste werden "
                        "still."}},
    [D_ENC_OFFER_SHARDS] = {1,
                            {"Du haeltst die Scherben hin. Der\nKami sieht sie an. "
                             "Etwas knackt\nwie brechendes Holz."}},
    [D_ENC_FIGHT] = {1, {"Du gehst auf den Kami los."}},
    [D_ENC_VICTORY] = {1,
                       {"Der Kami zerfaellt zu Laub und\nAsche. Im Hain wird es "
                        "still.\nSehr still."}},
    [D_ENC_DEFEAT] = {1,
                      {"Du kommst am Suedtor wieder zu\ndir. Jemand hat dich "
                       "gefunden\nund heimgebracht."}},
    [D_SUMI_FOUGHT] =
        {2,
         {"Du hast ihn vertrieben? Dann\nkoennen die Leute wieder\narbeiten. Gut.",
          "Komisch. Ich dachte, ich waere\nerleichtert."}},
    [D_SUMI_BOUNDARY] = {2,
                         {"Der alte Stein steht wieder da,\nwo er hingehoert? Dann "
                          "bleibt\nder Hain ihrer.",
                          "Daigo wird toben. Das Holz an\nder Grenze war das beste."}},
    [D_DAIGO_BOUNDARY] = {1,
                          {"Du hast den Stein zurueckgerollt.\nWeisst du, was das "
                           "kostet? Nein.\nWoher auch."}},
    [D_X_STONE_STUCK] = {1,
                         {"Der Stein steht schief im Weg.\nDu rollst ihn zurueck "
                          "an den\nAnfang der Schleifspur."}},
    [D_SCENE_BOUNDARY] = {2,
                          {"Der Stein rutscht in die Mulde,\nals haette er nie "
                           "woanders\ngelegen. Das Moos passt genau.",
                           "Im Hain wird es leiser. Etwas\nzieht sich zwischen die "
                           "Staemme\nzurueck und bleibt dort."}},
    [D_I_BOWL_MARK] = {1, {"Auf dem Boden einer Scherbe ist\nein Zeichen eingebrannt."}},
    [D_I_BOWL_MARK_MATCH] = {1,
                             {"Auf dem Boden einer Scherbe ist\nein Zeichen "
                              "eingebrannt. Wie\nneben Sumis Tuer."}},
};

/* At most 2 lines of 36 characters; no digits or slashes (test_game). */
const char *const notes[NOTE_COUNT] = {
    [NOTE_NONE] = "",
    [N_ASKED] = "Sumi bat mich, den Geist im Wald\nzu vertreiben.",
    [N_KENTA] = "Kenta sagt, der alte Baum habe\nihn angesehen.",
    [N_CLAW] = "Die Kratzspuren enden an den\nalten Steinen. Dahinter keine.",
    [N_DRAGGED] = "Eine Schleifspur. Jemand hat einen\nder Steine bewegt.",
    [N_HOLLOW] = "Eine Mulde mit Moosrand. Hier lag\nlange etwas Schweres.",
    [N_STUMPS] = "Frisches Harz. Diese Baeume\nfielen vor wenigen Tagen.",
    [N_ROPE] = "Um den alten Baum hing ein Seil.\nGerissen, nicht verrottet.",
    [N_INSCRIPTION] = "Am Schrein eingeritzt: Bis zu den\ndrei Steinen und nicht weiter.",
    [N_SHARDS] = "Scherben einer Opferschale. Darin\nvertrocknete Beeren.",
    [N_BOWL_MARK] = "Auf der Schale ein eingebranntes\nZeichen.",
    [N_HOUSE_MARK] = "Neben Sumis Tuer ein eingebranntes\nHauszeichen.",
    [N_MARKS_MATCH] = "Das Zeichen auf der Schale gleicht\ndem an Sumis Tuer.",
    [N_OWNER] = "Sumis Grossmutter gehoerte die\nSchale. Sie pflegte den Schrein.",
    [N_LEDGER] = "Ein Holzauftrag aus der Stadt.\nDie Frist ist knapp. Schulden.",
    [N_FOX] = "Ein verletzter Fuchs am Bau. An\nder Pfote dunkler Lehm.",
    [N_HERB] = "Mio gab mir Heilkraut fuer den\nFuchs.",
    [N_FOX_TENDED] = "Seit ich den Fuchs versorgt habe,\nsehe ich ueberall Faehrten.",
    [N_TRACKS] = "Tierspuren meiden das Lager und\nlaufen im Bogen um den Hain.",
    [N_KAMI] = "Etwas mit einem Geweih aus Aesten\nbewacht den Rand des Hains.",
    [N_KAMI_SHARDS] = "Als ich die Scherben zeigte, wurde\nder Kami zorniger.",
    [N_FOUGHT] = "Der Kami ist zerfallen. Im Hain\nist es sehr still.",
    [N_BOUNDARY] = "Der Stein liegt wieder in seiner\nMulde. Der Kami blieb im Hain.",
};

#define MARKS (OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK))
const DialogueRule dialogue_rules[] = {
    /* Reactions to an outcome come first. */
    {NPC_SUMI, 0, 0, 0, D_SUMI_FOUGHT, NOTE_NONE, ITEM_NONE, OUT_FIGHT},
    {NPC_SUMI, 0, 0, 0, D_SUMI_BOUNDARY, NOTE_NONE, ITEM_NONE, OUT_BOUNDARY},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO_BOUNDARY, NOTE_NONE, ITEM_NONE, OUT_BOUNDARY},
    {NPC_SUMI, MARKS, OBS(OBS_BOWL_OWNER), OBS(OBS_BOWL_OWNER), D_SUMI_OWNER, N_OWNER,
     ITEM_NONE, OUT_NONE},
    {NPC_SUMI, OBS(OBS_BOWL_OWNER), 0, 0, D_SUMI_OWNER_KNOWN, NOTE_NONE, ITEM_NONE,
     OUT_NONE},
    {NPC_SUMI, OBS(OBS_ASKED_BY_SUMI), 0, 0, D_SUMI_WAITING, NOTE_NONE, ITEM_NONE,
     OUT_NONE},
    {NPC_SUMI, 0, 0, OBS(OBS_ASKED_BY_SUMI), D_SUMI_TASK, N_ASKED, ITEM_NONE, OUT_NONE},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS) | OBS(OBS_BOWL_OWNER), 0, 0, D_ORIHA_OWNER,
     NOTE_NONE, ITEM_NONE, OUT_NONE},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS), 0, 0, D_ORIHA_SHARDS, NOTE_NONE, ITEM_NONE,
     OUT_NONE},
    {NPC_ORIHA, 0, 0, 0, D_ORIHA, NOTE_NONE, ITEM_NONE, OUT_NONE},
    {NPC_MIO, OBS(OBS_FOX_TENDED), 0, 0, D_MIO_THANKS, NOTE_NONE, ITEM_NONE, OUT_NONE},
    /* Mio keeps helping while the fox is hurt: the herb can be used up elsewhere. */
    {NPC_MIO, OBS(OBS_FOX_WOUNDED), OBS(OBS_MIO_HERB), OBS(OBS_MIO_HERB), D_MIO_HERB,
     N_HERB, ITEM_HERB, OUT_NONE},
    {NPC_MIO, OBS(OBS_FOX_WOUNDED), 0, 0, D_MIO_HERB_MORE, NOTE_NONE, ITEM_HERB,
     OUT_NONE},
    {NPC_MIO, OBS(OBS_MIO_HERB), 0, 0, D_MIO_HERB_AGAIN, NOTE_NONE, ITEM_NONE, OUT_NONE},
    {NPC_MIO, 0, 0, 0, D_MIO, NOTE_NONE, ITEM_NONE, OUT_NONE},
    {NPC_KENTA, OBS(OBS_KENTA_STARE), 0, 0, D_KENTA_AGAIN, NOTE_NONE, ITEM_NONE,
     OUT_NONE},
    {NPC_KENTA, 0, 0, OBS(OBS_KENTA_STARE), D_KENTA, N_KENTA, ITEM_NONE, OUT_NONE},
    {NPC_DAIGO, OBS(OBS_LEDGER_DEBT), 0, 0, D_DAIGO_LEDGER, NOTE_NONE, ITEM_NONE,
     OUT_NONE},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO, NOTE_NONE, ITEM_NONE, OUT_NONE},
};
const int dialogue_rule_count = (int)(sizeof dialogue_rules / sizeof dialogue_rules[0]);

const ExaminePoint examine_points[] = {
    /* Village */
    {.kind = POINT_AT,
     .map = MAP_VILLAGE,
     .x = 4,
     .y = 6,
     .needs = OBS(OBS_BOWL_MARK),
     .grants = OBS(OBS_HOUSE_MARK),
     .dialogue = D_X_HOUSE_MARK_MATCH,
     .note = N_MARKS_MATCH},
    {.kind = POINT_AT,
     .map = MAP_VILLAGE,
     .x = 4,
     .y = 6,
     .grants = OBS(OBS_HOUSE_MARK),
     .dialogue = D_X_HOUSE_MARK,
     .note = N_HOUSE_MARK},
    {.kind = POINT_SYMBOL, .map = MAP_VILLAGE, .symbol = 'W', .dialogue = D_X_WOOD},
    /* Forest: the moved stone comes before the generic stone rule. */
    /* The moved stone, wherever it stands: stuck away from both ends, it is
     * rolled back to the drag marks instead. */
    {.kind = POINT_STONE, .needs = OBS(OBS_STONE_MOVED), .dialogue = D_X_STONE_STUCK},
    {.kind = POINT_STONE,
     .grants = OBS(OBS_STONE_DRAGGED),
     .dialogue = D_X_DRAGGED,
     .note = N_DRAGGED},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'd',
     .grants = OBS(OBS_STONE_DRAGGED),
     .dialogue = D_X_DRAGGED,
     .note = N_DRAGGED},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'G',
     .grants = OBS(OBS_CLAW_MARKS_EDGE),
     .dialogue = D_X_CLAW,
     .note = N_CLAW},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'm',
     .grants = OBS(OBS_STONE_HOLLOW),
     .dialogue = D_X_HOLLOW,
     .note = N_HOLLOW},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'x',
     .grants = OBS(OBS_FRESH_STUMPS),
     .dialogue = D_X_STUMPS,
     .note = N_STUMPS},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'Y',
     .grants = OBS(OBS_BROKEN_ROPE),
     .dialogue = D_X_ROPE,
     .note = N_ROPE},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'O',
     .grants = OBS(OBS_SHRINE_INSCRIPTION),
     .dialogue = D_X_INSCRIPTION,
     .note = N_INSCRIPTION},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'o',
     .needs = OBS(OBS_BOWL_SHARDS),
     .dialogue = D_X_ALTAR_EMPTY},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'o',
     .gives = ITEM_SHARDS,
     .grants = OBS(OBS_BOWL_SHARDS),
     .dialogue = D_X_SHARDS,
     .note = N_SHARDS},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'k',
     .grants = OBS(OBS_LEDGER_DEBT),
     .dialogue = D_X_LEDGER,
     .note = N_LEDGER},
    {.kind = POINT_SYMBOL, .map = MAP_FOREST, .symbol = 'A', .dialogue = D_X_TENT},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'F',
     .grants = OBS(OBS_FOX_WOUNDED),
     .dialogue = D_X_FOX,
     .note = N_FOX},
    {.kind = POINT_SYMBOL, .map = MAP_FOREST, .symbol = 'f', .dialogue = D_X_FOX_TENDED},
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 't',
     .needs = OBS(OBS_FOX_TENDED),
     .grants = OBS(OBS_TRACKS),
     .dialogue = D_X_TRACKS,
     .note = N_TRACKS},
    /* Items */
    {.kind = POINT_ITEM,
     .map = MAP_FOREST,
     .symbol = 'F',
     .item = ITEM_HERB,
     .takes = ITEM_HERB,
     .needs = OBS(OBS_FOX_WOUNDED),
     .grants = OBS(OBS_FOX_TENDED),
     .dialogue = D_I_TEND_FOX,
     .note = N_FOX_TENDED},
    {.kind = POINT_ITEM,
     .item = ITEM_SHARDS,
     .needs = OBS(OBS_HOUSE_MARK),
     .grants = OBS(OBS_BOWL_MARK),
     .dialogue = D_I_BOWL_MARK_MATCH,
     .note = N_MARKS_MATCH},
    {.kind = POINT_ITEM,
     .item = ITEM_SHARDS,
     .grants = OBS(OBS_BOWL_MARK),
     .dialogue = D_I_BOWL_MARK,
     .note = N_BOWL_MARK},
};
const int examine_point_count = (int)(sizeof examine_points / sizeof examine_points[0]);

#define TRACK(x, y) {MAP_FOREST, x, y, 't', OBS(OBS_FOX_TENDED)}
const TileOverride tile_overrides[] = {
    {MAP_FOREST, 5, 20, 'f', OBS(OBS_FOX_TENDED)},
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
    {ENC_WAIT, "STEHEN BLEIBEN", 0, OPT_PEACE},
    {ENC_OFFER, "DARBRINGEN", 0, OPT_PEACE},
    {ENC_ATTACK, "ANGREIFEN", 0, OPT_BOTH},
    {ENC_HEAL, "HEILKRAUT NEHMEN", 0, OPT_FIGHT},
    {ENC_RETREAT, "ZURUECKWEICHEN", 0, OPT_BOTH},
};
const int encounter_option_count =
    (int)(sizeof encounter_options / sizeof encounter_options[0]);
_Static_assert(sizeof encounter_options / sizeof encounter_options[0] <=
                   ENCOUNTER_OPTION_LIMIT,
               "raise ENCOUNTER_OPTION_LIMIT");
const EncounterOffer encounter_offers[] = {
    {ITEM_SHARDS, 0, OBS(OBS_KAMI_ANGERED), MOOD_ANGRY, D_ENC_OFFER_SHARDS,
     N_KAMI_SHARDS},
};
const int encounter_offer_count =
    (int)(sizeof encounter_offers / sizeof encounter_offers[0]);
