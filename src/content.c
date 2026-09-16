#include "content.h"
const char *const obs_names[OBS_COUNT] = {
    "ASKED_BY_SUMI", "KENTA_STARE",        "CLAW_MARKS_EDGE", "FRESH_STUMPS",
    "BROKEN_ROPE",   "SHRINE_INSCRIPTION", "STONE_DRAGGED",   "STONE_HOLLOW",
    "BOWL_SHARDS",   "BOWL_MARK",          "HOUSE_MARK",      "BOWL_OWNER",
    "LEDGER_DEBT"};

const Npc npcs[NPC_COUNT] = {{MAP_VILLAGE, 5, 4, 0, "SUMI", "Sumi / Dorfaelteste"},
                             {MAP_VILLAGE, 24, 4, 1, "ORIHA", "Oriha / Lackmeisterin"},
                             {MAP_VILLAGE, 11, 10, 2, "MIO", "Mio"},
                             {MAP_VILLAGE, 18, 2, 3, "KENTA", "Kenta / Holzfaeller"},
                             {MAP_FOREST, 11, 31, 4, "DAIGO", "Daigo / Vorarbeiter"}};

const Transition transitions[TRANSITION_COUNT] = {
    {MAP_VILLAGE, 16, 0, MAP_FOREST, 24, 38}, {MAP_FOREST, 24, 39, MAP_VILLAGE, 16, 1}};

/* Pages: at most 3 lines of at most 36 characters (checked by test_game). */
const Dialogue dialogues[DIALOGUE_COUNT] = {
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
};

#define MARKS (OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK))
const DialogueRule dialogue_rules[] = {
    {NPC_SUMI, MARKS, OBS(OBS_BOWL_OWNER), OBS(OBS_BOWL_OWNER), D_SUMI_OWNER, N_OWNER},
    {NPC_SUMI, OBS(OBS_BOWL_OWNER), 0, 0, D_SUMI_OWNER_KNOWN, NOTE_NONE},
    {NPC_SUMI, OBS(OBS_ASKED_BY_SUMI), 0, 0, D_SUMI_WAITING, NOTE_NONE},
    {NPC_SUMI, 0, 0, OBS(OBS_ASKED_BY_SUMI), D_SUMI_TASK, N_ASKED},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS) | OBS(OBS_BOWL_OWNER), 0, 0, D_ORIHA_OWNER,
     NOTE_NONE},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS), 0, 0, D_ORIHA_SHARDS, NOTE_NONE},
    {NPC_ORIHA, 0, 0, 0, D_ORIHA, NOTE_NONE},
    {NPC_MIO, 0, 0, 0, D_MIO, NOTE_NONE},
    {NPC_KENTA, OBS(OBS_KENTA_STARE), 0, 0, D_KENTA_AGAIN, NOTE_NONE},
    {NPC_KENTA, 0, 0, OBS(OBS_KENTA_STARE), D_KENTA, N_KENTA},
    {NPC_DAIGO, OBS(OBS_LEDGER_DEBT), 0, 0, D_DAIGO_LEDGER, NOTE_NONE},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO, NOTE_NONE},
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
    {.kind = POINT_AT,
     .map = MAP_FOREST,
     .x = 24,
     .y = 16,
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
    /* Items */
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
