"""Tests für tools/content_drift.py.

Lauffähig mit: python3 -m unittest tools.test_content_drift
oder:          python3 tools/test_content_drift.py
"""
import json
import os
import re
import sys
import unittest

# Direkt aufrufbar (python3 tools/test_content_drift.py) und als Modul
# (python3 -m unittest tools.test_content_drift): dafuer muss das Repo-Wurzel-
# verzeichnis im Suchpfad stehen.
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import tools.content_drift as cd  # noqa: E402
from tools.content_drift import load_data, compare, vergleiche  # noqa: E402


def _dialogue(key, pages):
    inner = ",\n       ".join('"%s"' % p for p in pages)
    return '    [%s] = {%d,\n       {%s}}' % (key, len(pages), inner)


def _dialogues_table(entries):
    body = ",\n".join(entries)
    return "const Dialogue dialogues[DIALOGUE_COUNT] = {\n%s\n};" % body


def _note(key, text):
    return '    [%s] = "%s"' % (key, text)


def _notes_table(entries):
    body = ",\n".join(entries)
    return 'const char *const notes[NOTE_COUNT] = {\n%s\n};' % body


def _npc(name):
    return '{0, 0, 0, \'X\', "%s"}' % name


def _npcs_table(names):
    body = ",\n".join(_npc(n) for n in names)
    return "const Npc npcs[NPC_COUNT] = {%s};" % body


def _item(name):
    return '{"%s", 0}' % name


def _items_table(names):
    body = ",\n".join(_item(n) for n in names)
    return 'const ItemDef items[ITEM_COUNT] = {\n%s\n};' % body


def _mood(name):
    return '"%s"' % name


def _moods_table(names):
    body = ", ".join(_mood(n) for n in names)
    return 'const char *const mood_names[MOOD_COUNT] = {%s};' % body


def _dialogues_only(text):
    return {"dialogues": text}


class TestNamensTabellen(unittest.TestCase):
    """npcs und items stehen als Strukturen da: je Eintrag zaehlt das letzte
    Textfeld. Ueber alle Literale zu zaehlen verschiebt die Zuordnung, weil der
    PC pro Figur zwei Zeichenketten fuehrt (Schluessel und Anzeigename)."""

    PC_NPCS = ('const Npc npcs[NPC_COUNT] = {{0, 5, 4, 0, "SUMI", "Sumi"},\n'
               '                             {0, 24, 4, 1, "ORIHA", "Oriha"}};')
    C64_NPCS = ("const Npc npcs[NPC_COUNT] = {{0, 5, 4, 'S', \"Sumi\"},\n"
                "                             {0, 24, 4, 'O', \"Oriha\"}};")

    def test_namen_werden_ueberhaupt_verglichen(self):
        bf = vergleiche({"npcs": self.PC_NPCS}, {"npcs": self.C64_NPCS})
        self.assertEqual(bf, [])

    def test_abweichender_name_wird_gefunden(self):
        c64 = self.C64_NPCS.replace("Oriha", "Oriha die Lackmeisterin")
        bf = vergleiche({"npcs": self.PC_NPCS}, {"npcs": c64})
        self.assertEqual([(f[0], f[2], f[1]) for f in bf], [("Text", "npcs", "1")])

    def test_item_name_wird_verglichen(self):
        pc = 'const ItemDef items[ITEM_COUNT] = {{"", 0}, {"Heilkraut", 8}};'
        c64 = 'const ItemDef items[ITEM_COUNT] = {{"", 0}, {"Kraeuterbund", 8}};'
        bf = vergleiche({"items": pc}, {"items": c64})
        self.assertEqual([(f[0], f[2], f[1]) for f in bf], [("Text", "items", "1")])


class TestVergleiche(unittest.TestCase):
    def test_gleicher_text_kein_befund(self):
        """1. Gleicher Text in beiden → kein Befund."""
        table = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        bf = vergleiche(_dialogues_only(table), _dialogues_only(table))
        self.assertEqual(bf, [])

    def test_unterschiedlicher_text_befund(self):
        """2. Unterschiedlicher Text → ein Befund, der die ID nennt."""
        pc = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        c64 = _dialogues_table([_dialogue("D_TEST", ["Hallo Erde"])])
        bf = vergleiche(_dialogues_only(pc), _dialogues_only(c64))
        self.assertEqual(len(bf), 1)
        self.assertEqual(bf[0][1], "D_TEST")

    def test_zeilenumbrueche_kein_befund(self):
        """3. Unterschied nur in Zeilenumbrüchen → kein Befund."""
        pc = _dialogues_table([_dialogue("D_TEST", ["Hallo\nWelt"])])
        c64 = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        bf = vergleiche(_dialogues_only(pc), _dialogues_only(c64))
        self.assertEqual(bf, [])

    def test_id_nur_pc_kein_befund(self):
        """4. ID nur im PC vorhanden → kein Befund."""
        pc = _dialogues_table([
            _dialogue("D_TEST", ["Hallo"]),
            _dialogue("D_EXTRA", ["Extra"]),
        ])
        c64 = _dialogues_table([_dialogue("D_TEST", ["Hallo"])])
        bf = vergleiche(_dialogues_only(pc), _dialogues_only(c64))
        self.assertEqual(bf, [])

    def test_seitenzahl_unterschied_befund(self):
        """5. Unterschiedliche Seitenzahl → Befund mit der Art 'Seitenzahl'."""
        pc = _dialogues_table([_dialogue("D_TEST", ["Seite 1", "Seite 2"])])
        c64 = _dialogues_table([_dialogue("D_TEST", ["Seite 1"])])
        bf = vergleiche(_dialogues_only(pc), _dialogues_only(c64))
        self.assertEqual(len(bf), 1)
        self.assertEqual(bf[0][1], "D_TEST")
        self.assertEqual(bf[0][0], "Seitenzahl")

    def test_ausnahmeliste_kein_befund(self):
        """6. Eintrag in der Ausnahmeliste → kein Befund."""
        pc = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        c64 = _dialogues_table([_dialogue("D_TEST", ["Hallo Erde"])])
        ausnahmen = {"dialogues": {"D_TEST": "Grund"}}
        bf = vergleiche(_dialogues_only(pc), _dialogues_only(c64), ausnahmen)
        self.assertEqual(bf, [])

    def test_veralteter_eintrag_befund(self):
        """7. Ausnahmeliste enthält eine ID, die gar nicht abweicht → Befund."""
        table = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        ausnahmen = {"dialogues": {"D_TEST": "Grund"}}
        bf = vergleiche(_dialogues_only(table), _dialogues_only(table), ausnahmen)
        self.assertEqual(len(bf), 1)
        self.assertEqual(bf[0][0], "VeralteterEintrag")

    def test_unbekannter_eintrag_befund(self):
        """Eine ID in der Ausnahmeliste, die es in keiner Fassung gibt, ist ein
        Befund -- sonst deckt ein Tippfehler stillschweigend nichts ab."""
        table = _dialogues_table([_dialogue("D_TEST", ["Hallo Welt"])])
        ausnahmen = {"dialogues": {"D_TIPPFEHLER": "Grund"}}
        bf = vergleiche(_dialogues_only(table), _dialogues_only(table), ausnahmen)
        self.assertEqual(len(bf), 1)
        self.assertEqual(bf[0][0], "UnbekannterEintrag")
        self.assertEqual(bf[0][1], "D_TIPPFEHLER")

    def test_echte_dateien_kein_befund(self):
        """8. Ein Lauf gegen die echten Dateien des Repos → kein Befund."""
        repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        # Durch _read: die geteilten Texte stehen in shared/*.inc, nicht in content.c.
        pc_content = cd._read(os.path.join(repo, "src", "content.c"))
        pc_items = cd._read(os.path.join(repo, "src", "inventory.c"))
        c64_content = cd._read(os.path.join(repo, "c64", "src", "content.c"))
        with open(
            os.path.join(repo, "tools", "content_drift_allowlist.json"),
            encoding="utf-8",
        ) as f:
            ausnahmen = json.load(f)
        bf = vergleiche(
            {"dialogues": pc_content, "notes": pc_content,
             "npcs": pc_content, "items": pc_items,
             "mood_names": pc_content},
            {"dialogues": c64_content, "notes": c64_content,
             "npcs": c64_content, "items": c64_content,
             "mood_names": c64_content},
            ausnahmen,
        )
        self.assertEqual(bf, [])


# --- Regeltabellen -----------------------------------------------------------

PC_HEADER = """
enum { MAP_VILLAGE, MAP_FOREST };
typedef enum { ITEM_NONE, ITEM_HERB, ITEM_COUNT } ItemId;
typedef enum { NOTE_NONE, N_EINS, N_ZWEI, NOTE_COUNT } NoteId;
typedef enum { OUT_NONE, OUT_FIGHT, OUTCOME_COUNT } Outcome;
typedef enum { PHASE_BEFORE, PHASE_MORNING, PHASE_COUNT } Phase;
typedef struct {
  int npc;
  Obs needs, grants;
  int dialogue;
  NoteId note;
  ItemId gives;
  Outcome outcome;
  Phase phase;
} DialogueRule;
typedef struct {
  int map, x, y;
  char symbol;
  Obs needs;
  Outcome outcome;
} TileOverride;
"""

C64_HEADER = """
enum { MAP_VILLAGE, MAP_FOREST, MAP_COUNT };
typedef enum { ITEM_NONE, ITEM_HERB, ITEM_COUNT } ItemId;
typedef enum { NOTE_NONE, N_EINS, N_ZWEI, NOTE_COUNT } NoteId;
typedef enum { OUT_NONE, OUT_FIGHT, OUTCOME_COUNT } Outcome;
typedef struct {
  uint8_t npc;
  Obs needs, grants;
  uint8_t dialogue, note, gives, outcome;
} DialogueRule;
typedef struct {
  uint8_t map, x, y;
  char symbol;
  Obs needs;
  uint8_t outcome;
} TileOverride;
"""


def _rules_table(entries):
    body = ",\n".join(entries)
    return "const DialogueRule dialogue_rules[] = {\n%s\n};" % body


def _overrides_table(entries):
    body = ",\n".join(entries)
    return "const TileOverride tile_overrides[] = {\n%s\n};" % body


def _regelbefunde(pc_body, c64_body, ausnahmen=None, tabelle="dialogue_rules"):
    return vergleiche({tabelle: pc_body}, {tabelle: c64_body}, ausnahmen or {},
                      PC_HEADER, C64_HEADER)


class TestRegelTabellen(unittest.TestCase):
    """Die Regeln sagen, wer wann was sagt. Gleiche Texte reichen nicht."""

    def test_gleiche_regel_kein_befund(self):
        """9. Benannte und positionale Schreibweise derselben Regel."""
        pc = _rules_table(
            ['    {.npc = 1, .needs = OBS(A), .dialogue = D_EINS,'
             ' .note = N_EINS}'])
        c64 = _rules_table(['    {1, OBS(A), 0, D_EINS, N_EINS, 0, 0}'])
        self.assertEqual(_regelbefunde(pc, c64), [])

    def test_abweichende_bedingung_wird_gefunden(self):
        """10. Dieselbe Regel, andere Voraussetzung."""
        pc = _rules_table(
            ['    {.npc = 1, .needs = OBS(A), .dialogue = D_EINS}'])
        c64 = _rules_table(['    {1, OBS(B), 0, D_EINS, NOTE_NONE, 0, 0}'])
        bf = _regelbefunde(pc, c64)
        self.assertEqual([(b[0], b[1]) for b in bf],
                         [("Feld", "D_EINS|needs")])

    def test_nur_eine_fassung_kein_befund(self):
        """11. Eine Regel, die es nur auf dem PC gibt, ist kein Befund."""
        pc = _rules_table(['    {.npc = 1, .dialogue = D_EINS}',
                           '    {.npc = 1, .dialogue = D_NUR_PC}'])
        c64 = _rules_table(['    {1, 0, 0, D_EINS, NOTE_NONE, 0, 0}'])
        self.assertEqual(_regelbefunde(pc, c64), [])

    def test_nur_pc_kennt_das_feld_kein_befund(self):
        """12. Phase gibt es nur auf dem PC -- nichts zu vergleichen."""
        pc = _rules_table(['    {.npc = 1, .dialogue = D_EINS,'
                           ' .phase = PHASE_MORNING}'])
        c64 = _rules_table(['    {1, 0, 0, D_EINS, NOTE_NONE, 0, 0}'])
        self.assertEqual(_regelbefunde(pc, c64), [])

    def test_ausgelassenes_feld_ist_null(self):
        """13. Ausgelassen heisst null -- und ITEM_NONE heisst auch null."""
        pc = _rules_table(['    {.npc = 1, .dialogue = D_EINS}'])
        c64 = _rules_table(['    {1, 0, 0, D_EINS, NOTE_NONE, 0, 0}'])
        self.assertEqual(_regelbefunde(pc, c64), [])

    def test_ausnahme_braucht_grund(self):
        """14. Erlaubt ohne Grund bleibt ein Befund."""
        pc = _rules_table(
            ['    {.npc = 1, .needs = OBS(A), .dialogue = D_EINS}'])
        c64 = _rules_table(['    {1, OBS(B), 0, D_EINS, NOTE_NONE, 0, 0}'])
        mit_grund = {"dialogue_rules": {"D_EINS|needs": "so gewollt"}}
        self.assertEqual(_regelbefunde(pc, c64, mit_grund), [])
        ohne = {"dialogue_rules": {"D_EINS|needs": ""}}
        bf = _regelbefunde(pc, c64, ohne)
        self.assertEqual([b[0] for b in bf], ["FehlenderGrund"])

    def test_veraltete_ausnahme_wird_gemeldet(self):
        """15. Erlaubnis ohne Abweichung: der Eintrag ist ueberfluessig."""
        pc = _rules_table(['    {.npc = 1, .dialogue = D_EINS}'])
        c64 = _rules_table(['    {1, 0, 0, D_EINS, NOTE_NONE, 0, 0}'])
        bf = _regelbefunde(
            pc, c64, {"dialogue_rules": {"D_EINS|needs": "Grund"}})
        self.assertEqual([b[0] for b in bf], ["VeralteterEintrag"])

    def test_makros_werden_eingesetzt(self):
        """16. MARK(x, y) muss aufgeloest werden, sonst faellt nichts auf."""
        pc = ("#define MARK(x, y) {MAP_FOREST, x, y, 'P', OBS(A), OUT_ANY}\n"
              + _overrides_table(["    MARK(14, 14)"]))
        c64 = ("#define MARK(x, y) {MAP_FOREST, x, y, 'Q', OBS(A), OUT_ANY}\n"
               + _overrides_table(["    MARK(14, 14)"]))
        bf = _regelbefunde(pc, c64, tabelle="tile_overrides")
        self.assertEqual(
            [(b[0], b[1]) for b in bf],
            [("Feld", "MAP_FOREST:14:14:OBS(A):OUT_ANY|symbol")])

    def test_andere_bedingung_ist_ein_anderer_eintrag(self):
        """17. Die Kehrseite: bei Ueberschreibungen ist die Bedingung der
        Schluessel. Driftet sie, findet der Eintrag drueben keinen Partner
        und faellt aus dem Vergleich -- hier festgehalten, damit es eine
        bewusste Entscheidung bleibt."""
        pc = _overrides_table(["    {MAP_FOREST, 1, 2, 'P', OBS(A), OUT_ANY}"])
        c64 = _overrides_table(
            ["    {MAP_FOREST, 1, 2, 'P', OBS(B), OUT_ANY}"])
        self.assertEqual(_regelbefunde(pc, c64, tabelle="tile_overrides"), [])

    def test_text_ohne_regel_wird_gemeldet(self):
        """18. Steht ein Text in beiden Fassungen, muss ihn auch in beiden
        eine Regel zeigen -- sonst ist er unerreichbar."""
        dialoge = _dialogues_table([_dialogue("D_EINS", ["Hallo"]),
                                    _dialogue("D_ZWEI", ["Welt"])])
        pc = _rules_table(['    {.npc = 1, .dialogue = D_EINS}',
                           '    {.npc = 1, .dialogue = D_ZWEI}'])
        c64 = _rules_table(['    {1, 0, 0, D_EINS, NOTE_NONE, 0, 0}'])
        bf = vergleiche({"dialogues": dialoge, "dialogue_rules": pc},
                        {"dialogues": dialoge, "dialogue_rules": c64},
                        {}, PC_HEADER, C64_HEADER)
        self.assertEqual([(b[0], b[1], b[3]) for b in bf],
                         [("OhneRegel", "D_ZWEI", "C64")])

    def test_fehlende_tabelle_bricht_nicht_ab(self):
        """20. Fehlt eine Regeltabelle in beiden Fassungen, muss der Bericht
        das sagen koennen statt mit einem Traceback zu enden."""
        repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        weg = re.compile(r"const Place places\[\][^;]*;", re.S)
        def ohne_orte(pfad):
            return weg.sub("", cd._read(os.path.join(repo, *pfad)))
        h_pc = "\n".join(open(f, encoding="utf-8").read() for f in cd.PC_HEADERS)
        h_c64 = "\n".join(open(f, encoding="utf-8").read() for f in cd.C64_HEADERS)
        with open(os.path.join(repo, "src", "inventory.c"), encoding="utf-8") as f:
            items = f.read()
        data = cd.parse_texts(ohne_orte(("src", "content.c")), items,
                              ohne_orte(("c64", "src", "content.c")), h_pc, h_c64)
        befunde, zusammenfassung = cd.compare(data, cd.load_allowlist())
        self.assertEqual(zusammenfassung["places"], 0)
        self.assertEqual(cd._print_results(befunde, zusammenfassung), 0)

    def test_erwaehnung_ist_keine_definition(self):
        """21. `sizeof places[0]` erwaehnt die Tabelle, definiert sie nicht."""
        self.assertFalse(cd._defines_array(
            "const int place_count = (int)(sizeof places / sizeof places[0]);",
            "places"))
        self.assertTrue(cd._defines_array(
            "const Place places[] = {{0, 0, 0, 1, 1, \"X\", 0}};", "places"))

    def test_echte_regeln_kein_befund(self):
        """19. Die echten Dateien des Repos, mit der Erlaubnisliste."""
        repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        with open(os.path.join(repo, "tools", "content_drift_allowlist.json"),
                  encoding="utf-8") as f:
            ausnahmen = json.load(f)
        befunde, _ = compare(load_data(), ausnahmen)
        self.assertEqual(befunde, [])


if __name__ == "__main__":
    unittest.main()
