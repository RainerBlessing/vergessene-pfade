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
    [D_NONE] = {0, {0}},
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
    [D_SUMI_FOUGHT] = {2,
                       {"Du hast ihn vertrieben? Dann\nkoennen die Leute wieder\n"
                        "arbeiten. Gut.",
                        "Komisch. Ich dachte, ich waere\nerleichtert."}},
    [D_SUMI_MEND] = {2,
                     {"Ihr habt eine neue Grenze\nabgesteckt? Und der Hain laesst\n"
                      "uns das Totholz?",
                      "Wir werden sparsam heizen muessen.\nUnd jedes Jahr eine Gabe "
                      "bringen.\nGrossmutter haette gelacht."}},
    [D_SUMI_BOUNDARY] = {2,
                         {"Der alte Stein steht wieder da,\nwo er hingehoert? Dann "
                          "bleibt\nder Hain ihrer.",
                          "Daigo wird toben. Das Holz an\nder Grenze war das beste."}},
    [D_ORIHA] = {2,
                 {"Ich bin Oriha. Ich flicke, was\nzerbrochen ist. Mit Lack und\nGold.",
                  "Aber ich flicke nichts, dessen\nGeschichte ich nicht kenne."}},
    [D_ORIHA_SHARDS] = {1,
                        {"Scherben einer Opferschale.\nWem gehoerte sie? Ohne "
                         "ihre\nGeschichte fasse ich sie nicht an."}},
    [D_ORIHA_MARK] = {1,
                      {"Ein Hauszeichen, kein Zeichen\neiner Toepferei. Frag die "
                       "Leute,\nan deren Tuer es steht."}},
    [D_ORIHA_MARK_MATCH] = {1,
                            {"An Sumis Tuer steht es, ja.\nDann frag sie nach "
                             "der Schale."}},
    [D_ORIHA_MEND] = {2,
                      {"Sumis Grossmutter also. Dann hat\ndiese Schale eine "
                       "Geschichte,\ndie ich kenne.",
                       "Setz dich. Ich hole den Goldlack.\nDu setzt die Stuecke, "
                       "ich fuehre\ndeine Hand."}},
    [D_ORIHA_DRYING] = {1,
                        {"Der Lack braucht Ruhe. Komm\nwieder, wenn du aus dem "
                         "Wald\nzurueck bist."}},
    [D_ORIHA_READY] = {2,
                       {"Sie ist trocken. Sieh nur, die\nNaehte glaenzen. Der Bruch "
                        "ist\njetzt Teil der Schale.",
                        "Nimm sie mit. Sie gehoert nicht\nin mein Regal."}},
    [D_ORIHA_AFTER] = {1,
                       {"Trag sie behutsam. Und bring sie\ndorthin, wo sie gebraucht "
                        "wird."}},
    [D_MEND_WRONG] = {1, {"Das passt nicht an diese\nBruchkante."}},
    [D_MEND_DONE] = {1,
                     {"Das letzte Stueck sitzt. Der Lack\nzieht eine goldene Naht "
                      "durch\njeden Sprung."}},
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
    [D_MIO_THANKS] = {1, {"Du hast ihm geholfen? Danke!\nFuechse merken sich so "
                          "etwas."}},
    [D_KENTA] = {2,
                 {"Ich geh da nicht mehr rein! Der\nalte Baum hat mich angeschaut.\n"
                  "Mit ... mit Rinde!",
                  "Daigo sagt, ich soll mich nicht\nanstellen. Aber ihn hat es "
                  "auch\nerwischt. Ha."}},
    [D_KENTA_AGAIN] = {1, {"Nein. Nicht mal fuer Reiskuchen.\nDer Baum GUCKT."}},
    [D_DAIGO] = {2,
                 {"Noch ein Neugieriger? Der Geist\nhat drei meiner Leute "
                  "verletzt.\nMich eingeschlossen.",
                  "Ich brauche das Holz. Frag nicht\nwarum. Sorg einfach dafuer,\n"
                  "dass wir arbeiten koennen."}},
    [D_DAIGO_LEDGER] = {1,
                        {"Du hast in mein Buch gesehen?\nJa. Liefere ich nicht, "
                         "verliert\ndas Lager alles."}},
    [D_DAIGO_FOUGHT] = {1,
                        {"Der Geist ist fort? Dann koennen\nwir wieder arbeiten. Ich "
                         "schicke\ndie Leute morgen in den Wald."}},
    [D_DAIGO_BOUNDARY] = {1,
                          {"Du hast den Stein zurueckgerollt.\nWeisst du, was das "
                           "kostet? Nein.\nWoher auch."}},
    [D_DAIGO_MEND] = {1,
                      {"Bis zu den Pfaehlen also. Das ist\nweniger Holz. Aber es "
                       "ist Holz,\num das niemand streitet."}},
    [D_DAIGO_CALM] = {1,
                      {"Ein Geist, der sitzt und nichts tut?\nDann zeig mir, wo "
                       "meine Leute\ngefahrlos schlagen koennen."}},
    [D_DAIGO_OFFER] = {2,
                       {"Es sitzt neben einer Schale und\nruehrt sich nicht? Und die "
                        "Tiere\nlaufen alle diesen Bogen?",
                        "Gut. Zeig mir, wo die Spuren\nlaufen. Dort stecken wir die "
                        "neue\nGrenze ab. Ich komme mit."}},
    [D_DAIGO_WALKING] = {1,
                         {"Geh voran. Drei Pfaehle, dann ist\nSchluss mit dem "
                          "Streit."}},
    [D_STAKE_SET] = {1, {"Ihr treibt einen Pfahl in den\nBoden, neben die Spuren."}},
    [D_X_WOOD] = {1, {"Kaum Holz auf dem Stapel.\nUnd der Winter kommt bald."}},
    [D_X_WOOD_FIGHT] = {1,
                        {"Der Stapel waechst. Holz aus dem\nHain, genug fuer den "
                         "Winter."}},
    [D_X_WOOD_BOUNDARY] = {1,
                           {"So leer wie gestern. Das Lager\nschlaegt nicht mehr."}},
    [D_X_WOOD_MEND] = {1,
                       {"Totholz vom Hainrand. Weniger, als\nder Auftrag verlangt."}},
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
    [D_X_STONE_STUCK] = {1,
                         {"Der Stein steht schief im Weg.\nDu rollst ihn zurueck "
                          "an den\nAnfang der Schleifspur."}},
    [D_X_HOLLOW] = {1, {"Eine Mulde mit Moosrand. Hier lag\nlange etwas Schweres."}},
    [D_X_STUMPS] = {1, {"Frisches Harz. Diese Baeume\nfielen vor wenigen Tagen."}},
    [D_X_ROPE] = {1,
                  {"Um den alten Baum hing ein Seil.\nEs ist gerissen, nicht\n"
                   "verrottet."}},
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
    [D_X_INN_SIGN] = {1, {"Ein Holzschild ueber der Tuer:\nGASTHAUS."}},
    [D_X_WORKSHOP_SIGN] = {1,
                           {"Ein Schild mit einer Schale und\neinem Pinsel: Orihas "
                            "Lackwerkstatt."}},
    [D_X_WORKBENCH] = {1,
                       {"Schalen, Pinsel, Lacknaepfe. Eine\nSchale traegt eine "
                        "goldene Naht."}},
    [D_I_TEND_FOX] = {2,
                      {"Du legst Mios Kraut auf die\nWunde. Der Fuchs haelt "
                       "still.\nDann humpelt er in den Bau.",
                       "Als du dich aufrichtest, faellt\ndir etwas am Boden auf: "
                       "Faehrten.\nUeberall Faehrten."}},
    [D_I_BOWL_MARK] = {1, {"Auf dem Boden einer Scherbe ist\nein Zeichen "
                           "eingebrannt."}},
    [D_I_BOWL_MARK_MATCH] = {1,
                             {"Auf dem Boden einer Scherbe ist\nein Zeichen "
                              "eingebrannt. Dasselbe\nwie an Sumis Tuer?"}},
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
    /* An empty page shows the last message: the numbers of the closing round. */
    /* Eine Seite: die Begegnungstafel blaettert nicht, dort zaehlt jede Zeile. */
    [D_ENC_OFFER_BOWL] = {1,
                          {"Du stellst die geflickte Schale ins\nMoos. Der Kami "
                           "beugt sich darueber.\nEs setzt sich neben die Schale."}},
    [D_ENC_VICTORY] = {2,
                       {"", "Der Kami zerfaellt zu Laub und\nAsche. Im Hain wird es "
                            "still.\nSehr still."}},
    [D_ENC_DEFEAT] = {2,
                      {"", "Du kommst am Nordtor wieder zu\ndir. Jemand hat dich "
                           "gefunden\nund heimgebracht."}},
    [D_SCENE_MEND] = {2,
                      {"Der dritte Pfahl steht. Daigo\ntritt die Erde fest und sagt "
                       "lange\nnichts.",
                       "Zwischen den Staemmen sitzt der\nKami bei der Schale. Es "
                       "sieht\neuch zu und bleibt sitzen."}},
    [D_SCENE_BOUNDARY] = {2,
                          {"Der Stein rutscht in die Mulde,\nals haette er nie "
                           "woanders\ngelegen. Das Moos passt genau.",
                           "Im Hain wird es leiser. Etwas\nzieht sich zwischen die "
                           "Staemme\nzurueck und bleibt dort."}},
};

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
    [N_MENDED] = "Oriha und ich haben die Schale\ngeflickt. Der Lack trocknet.",
    [N_BOWL_READY] = "Die Schale ist trocken. Die Naehte\nglaenzen golden.",
    [N_KAMI_CALM] = "Vor der geflickten Schale hat der\nKami sich hingesetzt.",
    [N_DEAL] = "Daigo will mit mir eine neue\nGrenze abstecken.",
    [N_MEND] = "Drei Pfaehle stehen entlang der\nSpuren. Der Kami blieb sitzen.",
};

#define MARKS (OBS(OBS_BOWL_MARK) | OBS(OBS_HOUSE_MARK))
const DialogueRule dialogue_rules[] = {
    /* The bowl's story outranks every reaction: it is the only source of N_OWNER. */
    {NPC_SUMI, MARKS, OBS(OBS_BOWL_OWNER), OBS(OBS_BOWL_OWNER), D_SUMI_OWNER, N_OWNER,
     ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    /* The task stays reachable before her reaction to an outcome. */
    {NPC_SUMI, 0, OBS(OBS_ASKED_BY_SUMI), OBS(OBS_ASKED_BY_SUMI), D_SUMI_TASK, N_ASKED,
     ITEM_NONE, OUT_NONE, OPEN_NOTHING},
    {NPC_SUMI, 0, 0, 0, D_SUMI_FOUGHT, NOTE_NONE, ITEM_NONE, OUT_FIGHT, OPEN_END},
    {NPC_SUMI, 0, 0, 0, D_SUMI_BOUNDARY, NOTE_NONE, ITEM_NONE, OUT_BOUNDARY, OPEN_END},
    {NPC_SUMI, 0, 0, 0, D_SUMI_MEND, NOTE_NONE, ITEM_NONE, OUT_MEND, OPEN_END},
    {NPC_SUMI, OBS(OBS_BOWL_OWNER), 0, 0, D_SUMI_OWNER_KNOWN, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_SUMI, OBS(OBS_ASKED_BY_SUMI), 0, 0, D_SUMI_WAITING, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    /* The bowl on her shelf: handed over once the lacquer has dried. */
    {NPC_ORIHA, OBS(OBS_BOWL_READY), 0, 0, D_ORIHA_READY, N_BOWL_READY, ITEM_BOWL,
     OUT_ANY, OPEN_NOTHING},
    {NPC_ORIHA, OBS(OBS_BOWL_READY), 0, 0, D_ORIHA_AFTER, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_ORIHA, OBS(OBS_BOWL_DRYING), 0, 0, D_ORIHA_DRYING, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS) | OBS(OBS_BOWL_OWNER), 0, 0, D_ORIHA_MEND,
     NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_MEND},
    /* Wer beide Zeichen verbunden hat, wird nicht noch einmal losgeschickt --
     * er bekommt die naechste Tuer genannt. Die Geschichte erzaehlt nur Sumi. */
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS) | MARKS, OBS(OBS_BOWL_OWNER), 0,
     D_ORIHA_MARK_MATCH, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    /* Nur das Zeichen auf der Scherbe gesehen: was fuer ein Zeichen es ist. */
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS) | OBS(OBS_BOWL_MARK), OBS(OBS_HOUSE_MARK), 0,
     D_ORIHA_MARK, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    {NPC_ORIHA, OBS(OBS_BOWL_SHARDS), 0, 0, D_ORIHA_SHARDS, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_ORIHA, 0, 0, 0, D_ORIHA, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    {NPC_MIO, OBS(OBS_FOX_TENDED), 0, 0, D_MIO_THANKS, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    /* Mio keeps helping while the fox is hurt: the herb can be used up elsewhere. */
    {NPC_MIO, OBS(OBS_FOX_WOUNDED), OBS(OBS_MIO_HERB), OBS(OBS_MIO_HERB), D_MIO_HERB,
     N_HERB, ITEM_HERB, OUT_ANY, OPEN_NOTHING},
    {NPC_MIO, OBS(OBS_FOX_WOUNDED), 0, 0, D_MIO_HERB_MORE, NOTE_NONE, ITEM_HERB,
     OUT_ANY, OPEN_NOTHING},
    {NPC_MIO, OBS(OBS_MIO_HERB), 0, 0, D_MIO_HERB_AGAIN, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    {NPC_MIO, 0, 0, 0, D_MIO, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    {NPC_KENTA, OBS(OBS_KENTA_STARE), 0, 0, D_KENTA_AGAIN, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_KENTA, 0, 0, OBS(OBS_KENTA_STARE), D_KENTA, N_KENTA, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO_FOUGHT, NOTE_NONE, ITEM_NONE, OUT_FIGHT, OPEN_NOTHING},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO_BOUNDARY, NOTE_NONE, ITEM_NONE, OUT_BOUNDARY, OPEN_NOTHING},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO_MEND, NOTE_NONE, ITEM_NONE, OUT_MEND, OPEN_NOTHING},
    /* The compromise: he comes along once the kami sits and the tracks are known. */
    {NPC_DAIGO, OBS(OBS_KAMI_CALMED) | OBS(OBS_TRACKS) | OBS(OBS_LEDGER_DEBT),
     OBS(OBS_DAIGO_DEAL), OBS(OBS_DAIGO_DEAL), D_DAIGO_OFFER, N_DEAL, ITEM_NONE,
     OUT_NONE, OPEN_FOLLOW},
    {NPC_DAIGO, OBS(OBS_DAIGO_DEAL), 0, 0, D_DAIGO_WALKING, NOTE_NONE, ITEM_NONE,
     OUT_NONE, OPEN_FOLLOW},
    /* Der Kami sitzt, aber Daigo fehlt noch etwas: Er sagt, was er braucht,
     * ohne zu sagen, wo es steht. */
    {NPC_DAIGO, OBS(OBS_KAMI_CALMED), OBS(OBS_DAIGO_DEAL), 0, D_DAIGO_CALM, NOTE_NONE,
     ITEM_NONE, OUT_NONE, OPEN_NOTHING},
    {NPC_DAIGO, OBS(OBS_LEDGER_DEBT), 0, 0, D_DAIGO_LEDGER, NOTE_NONE, ITEM_NONE,
     OUT_ANY, OPEN_NOTHING},
    {NPC_DAIGO, 0, 0, 0, D_DAIGO, NOTE_NONE, ITEM_NONE, OUT_ANY, OPEN_NOTHING},
};
const uint8_t dialogue_rule_count =
    (uint8_t)(sizeof dialogue_rules / sizeof dialogue_rules[0]);

const ExaminePoint examine_points[] = {
    /* Village */
    {POINT_AT, MAP_VILLAGE, 4, 6, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE,
     OBS(OBS_BOWL_MARK), OBS(OBS_HOUSE_MARK), D_X_HOUSE_MARK_MATCH, N_MARKS_MATCH, OUT_NONE},
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
    {POINT_STONE, MAP_FOREST, 0, 0, 0, ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_STONE_DRAGGED), D_X_DRAGGED, N_DRAGGED, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'd', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_STONE_DRAGGED), D_X_DRAGGED, N_DRAGGED, OUT_NONE},
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'G', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0,
     OBS(OBS_CLAW_MARKS_EDGE), D_X_CLAW, N_CLAW, OUT_NONE},
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
    {POINT_SYMBOL, MAP_FOREST, 0, 0, 'A', ITEM_NONE, ITEM_NONE, ITEM_NONE, 0, 0,
     D_X_TENT, NOTE_NONE, OUT_NONE},
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
    {ENC_WAIT, "STEHEN BLEIBEN", OPT_PEACE},
    {ENC_OFFER, "DARBRINGEN", OPT_PEACE},
    {ENC_ATTACK, "ANGREIFEN", OPT_BOTH},
    {ENC_HEAL, "HEILKRAUT NEHMEN", OPT_FIGHT},
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
    {MAP_VILLAGE, 2, 13, 8, 4, "Gasthaus von Kiriyama", 0},
    {MAP_VILLAGE, 21, 3, 8, 4, "Orihas Lackwerkstatt", 0},
    {MAP_VILLAGE, 2, 3, 8, 4, "Sumis Haus", OBS(OBS_ASKED_BY_SUMI)},
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
