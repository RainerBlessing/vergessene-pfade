"""Tests für tools/content_drift.py.

Lauffähig mit: python3 -m unittest tools.test_content_drift
oder:          python3 tools/test_content_drift.py
"""
import json
import os
import sys
import unittest

# Direkt aufrufbar (python3 tools/test_content_drift.py) und als Modul
# (python3 -m unittest tools.test_content_drift): dafuer muss das Repo-Wurzel-
# verzeichnis im Suchpfad stehen.
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools.content_drift import vergleiche  # noqa: E402


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
        with open(os.path.join(repo, "src", "content.c"), encoding="utf-8") as f:
            pc_content = f.read()
        with open(os.path.join(repo, "src", "inventory.c"), encoding="utf-8") as f:
            pc_items = f.read()
        with open(os.path.join(repo, "c64", "src", "content.c"), encoding="utf-8") as f:
            c64_content = f.read()
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


if __name__ == "__main__":
    unittest.main()
