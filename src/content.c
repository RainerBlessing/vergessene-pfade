#include "content.h"
const Npc npcs[4] = {{1, 10, 7, 0, ROLE_KEEPER, "Aoi / Schreinhueterin"},
                     {1, 22, 9, 1, ROLE_MERCHANT, "Ren / Teehaendler"},
                     {1, 8, 15, 2, ROLE_RESIDENT, "Hana / Papiermacherin"},
                     {1, 19, 19, 3, ROLE_SCOUT, "Nao / Wegwaechterin"}};
const Transition transitions[2] = {{0, 10, 8, 1, 15, 27}, {1, 15, 28, 0, 10, 9}};
const Dialogue dialogues[7] = {
    {3,
     {"Willkommen in Koharu, Wanderer.\nUnsere Schreinlaterne ist erloschen.\nEin Geist hat den Spiegel gestohlen.",
      "Hilfst du uns? Nao steht am\nsuedlichen Torii. Frage sie nach\ndem Pfad durch den Bambuswald.",
      "Der Spiegel faengt das letzte\nSonnenlicht fuer die Laterne ein.\nBring ihn heim. Dafuer gibt es Gold."}},
    {2,
     {"Stell deine Sandalen an die Tuer.\nEine Tasse Beifusstee gibt dir\n8 Lebenspunkte fuer die Reise.",
      "Ich verkaufe auch eine Reiseklinge.\nWaehle deinen Proviant mit Bedacht.\nMoege dein Weg friedlich sein."}},
    {2,
     {"Ich schoepfe Papier fuer Laternen.\nWenn die Kirschblueten fallen,\nhaengen wir sie am Fluss auf.",
      "Ein kleines Licht in der Daemmerung\nweist einem Fremden den Heimweg.\nDeshalb mache ich immer weiter."}},
    {2,
     {"Verlasse das Dorf am Suedtor.\nGehe nach SUEDEN bis zur Gabelung,\ndann nach OSTEN ueber die Bruecke.",
      "Biege hinter der Bruecke NORDWAERTS\nab. Dort liegt der Waldschrein.\nEin Dornengeist bewacht den Spiegel."}},
    {1,
     {"Der Spiegel liegt am Waldschrein.\nNao kennt den Weg durch den Bambus.\nNimm vor deiner Reise etwas Tee mit."}},
    {2,
     {"Unser Bernsteinspiegel! Du hast ihn!\nHeute wird der Schrein leuchten.\nLaternen werden den Fluss saeumen.",
      "Nimm 25 Gold und ruhe dich aus.\nDeine Lebenskraft ist wieder voll.\nEMBERPOST / AUFTRAG ERFUELLT"}},
    {1,
     {"Die Laterne leuchtet wieder.\nVerweile unter den Kirschblueten.\nDu bist hier immer willkommen."}}};
