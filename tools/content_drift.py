#!/usr/bin/env python3
"""Vergleicht Spielinhalte zwischen der PC- und der C64-Umsetzung.

Liest beide content.c-Dateien und die PC-Items aus inventory.c,
normalisiert die Texte und meldet Abweichungen bei Dialogen,
Notizen, NPC-Namen, Item-Namen und Stimmungsbezeichnungen.
Abweichungen sind nur mit Eintrag in der Erlaubnisliste erlaubt;
das Skript erzwingt eine Entscheidung.
"""
import argparse
import json
import os
import re
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PC_CONTENT = os.path.join(REPO_ROOT, "src", "content.c")
PC_ITEMS = os.path.join(REPO_ROOT, "src", "inventory.c")
C64_CONTENT = os.path.join(REPO_ROOT, "c64", "src", "content.c")
ALLOWLIST = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                         "content_drift_allowlist.json")

TABLES = ("dialogues", "notes", "npcs", "items", "mood_names")
STRING_RE = r'"((?:[^"\\]|\\.)*)"'


def _read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


def _extract_array(text, name):
    """Gibt den Inhalt zwischen '{' und '};' eines Arrays zurueck."""
    m = re.search(r"\b" + re.escape(name) + r"\s*\[[^\]]*\]\s*=\s*\{", text)
    if not m:
        raise ValueError("Array %s nicht gefunden" % name)
    start = m.end()
    depth = 1
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i]
    raise ValueError("Array %s: schliessende Klammer fehlt" % name)


def _join_literals(s):
    """Verbindet benachbarte C-String-Literale zu einem einzigen."""
    pattern = re.compile(
        r'"((?:[^"\\]|\\.)*)"\s*"((?:[^"\\]|\\.)*)"', re.DOTALL)
    while True:
        s, n = pattern.subn(lambda m: '"' + m.group(1) + m.group(2) + '"', s,
                            count=1)
        if n == 0:
            return s


def normalize(text):
    """Normiert Text: benachbarte Literale verbinden, \\n zu Leerzeichen,
    Leerzeichen-Sequenzen auf ein Zeichen reduzieren."""
    text = _join_literals(text)
    text = text.replace("\\n", " ")
    return " ".join(text.split())


def _assign_entries(body):
    """Teilt Array-Inhalt in [KEY] = value Eintraege auf.

    Gibt Liste von (key, value_text) zurueck. Das Wert-Ende wird an
    dem naechsten [KEY] begrenzt, damit der Key-Prifix des Folgeeintrags
    nicht mitgelesen wird."""
    entries = []
    for m in re.finditer(r"\[\s*([A-Za-z_][A-Za-z0-9_]*)\s*\]\s*=", body):
        entries.append((m.group(1), m.end(), m.start()))
    for i, (key, start, _) in enumerate(entries):
        if i + 1 < len(entries):
            end = entries[i + 1][2]
            val = body[start:end]
            m = re.search(r"\[\s*[A-Za-z_][A-Za-z0-9_]*\s*\]", val)
            if m:
                val = val[:m.start()]
            entries[i] = (key, val.strip().rstrip(","))
        else:
            entries[i] = (key, body[start:].strip().rstrip(","))
    return entries


def _positional_strings(body):
    """Array ohne explizite Keys: gibt (index, text) Liste zurueck."""
    return [(str(i), s) for i, s in enumerate(re.findall(STRING_RE, body))]


def _brace_groups(body):
    """Die Eintraege eines Arrays aus Strukturen: je Text zwischen { und }.

    Wird fuer npcs und items gebraucht: Dort steht pro Eintrag mehr als eine
    Zeichenkette (PC: Schluessel und Anzeigename), und ueber alle Literale zu
    zaehlen verschiebt die Zuordnung zwischen den Fassungen."""
    groups, depth, start = [], 0, None
    for i, c in enumerate(body):
        if c == "{":
            if depth == 0:
                start = i + 1
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and start is not None:
                groups.append(body[start:i])
                start = None
    return [(str(i), g) for i, g in enumerate(groups)]


def _parse_plain(body):
    """Reines String-Array (notes, mood_names): key -> normierter Text."""
    if re.search(r"\[\s*[A-Za-z_]", body):
        items = _assign_entries(body)
    else:
        items = _positional_strings(body)
    return {key: normalize(val) for key, val in items}


def _parse_names(body):
    """Strukturiertes Array (npcs, items): key -> letztes String-Feld."""
    if re.search(r"\[\s*[A-Za-z_]", body):
        items = _assign_entries(body)
    else:
        items = _brace_groups(body)
    result = {}
    for key, val in items:
        fields = re.findall(STRING_RE, val)
        result[key] = normalize(fields[-1]) if fields else ""
    return result


def _parse_dialogues(body):
    """Dialog-Array: key -> (page_count, [normierte Seitentexte])."""
    result = {}
    for key, val in _assign_entries(body):
        outer = val[val.find("{"):val.rfind("}") + 1]
        count_m = re.match(r"\s*([0-9]+)\s*,", outer[1:])
        count = int(count_m.group(1)) if count_m else 0
        joined = _join_literals(outer)
        pages = [normalize(f) for f in re.findall(STRING_RE, joined)]
        result[key] = (count, pages)
    return result


def _parse_table(text, name):
    body = _extract_array(text, name)
    if name == "dialogues":
        return _parse_dialogues(body)
    if name in ("npcs", "items"):
        return _parse_names(body)
    return _parse_plain(body)


def parse_texts(pc_content_text, pc_items_text, c64_content_text):
    """Parst Quelltexte und gibt dict mit 'pc' und 'c64' zurueck.

    PC: dialogues, notes, npcs, mood_names aus pc_content_text;
        items aus pc_items_text.
    C64: alle Tabellen aus c64_content_text (items dort ebenfalls)."""
    data = {"pc": {}, "c64": {}}
    for name in TABLES:
        pc_src = pc_items_text if name == "items" else pc_content_text
        data["pc"][name] = _parse_table(pc_src, name)
        data["c64"][name] = _parse_table(c64_content_text, name)
    return data


def load_data():
    pc_content = _read(PC_CONTENT)
    pc_items_src = _read(PC_ITEMS)
    c64_content = _read(C64_CONTENT)
    return parse_texts(pc_content, pc_items_src, c64_content)


def load_allowlist(path=None):
    if path is None:
        path = ALLOWLIST
    if not os.path.exists(path):
        return {t: {} for t in TABLES}
    with open(path, encoding="utf-8") as f:
        raw = json.load(f)
    result = {}
    for table in TABLES:
        entries = raw.get(table, {})
        result[table] = {}
        for key, reason in entries.items():
            result[table][key] = reason if isinstance(reason, str) else ""
    return result


def _trunc(s, n=100):
    s = str(s).replace("\n", " ")
    return s if len(s) <= n else s[:n] + "..."


def vergleiche(pc_tables, c64_tables, ausnahmen=None):
    """Vergleicht Inhalte aus rohen Quelltexten.

    pc_tables/c64_tables: dict table -> Quelltext-Snippet
    ausnahmen: dict table -> {key: reason}

    Gibt nur die Liste der Befunde zurueck (ohne Zusammenfassung)."""
    if ausnahmen is None:
        ausnahmen = {}
    data = {"pc": {}, "c64": {}}
    for name in TABLES:
        pc_src = pc_tables.get(name, "")
        c64_src = c64_tables.get(name, "")
        data["pc"][name] = _parse_table(pc_src, name) if pc_src else {}
        data["c64"][name] = _parse_table(c64_src, name) if c64_src else {}
    findings, _ = compare(data, ausnahmen)
    return findings


def compare(data, allowlist):
    """Gibt Liste von Befunden und Zusammenfassung zurueck."""
    findings = []
    summary = {}

    for table in TABLES:
        pc = data["pc"][table]
        c64 = data["c64"][table]
        common = sorted(set(pc) & set(c64))
        summary[table] = len(common)

        for key in common:
            pc_v = pc[key]
            c64_v = c64[key]
            table_exceptions = allowlist.get(table, {})
            allowed = key in table_exceptions
            reason = table_exceptions.get(key, "")

            drifts = []
            if table == "dialogues":
                pc_count, pc_pages = pc_v
                c64_count, c64_pages = c64_v
                if pc_count != c64_count:
                    drifts.append(("Seitenzahl", str(pc_count), str(c64_count)))
                for i in range(min(pc_count, c64_count)):
                    if pc_pages[i] != c64_pages[i]:
                        drifts.append(("Text", pc_pages[i], c64_pages[i]))
            else:
                if pc_v != c64_v:
                    drifts.append(("Text", pc_v, c64_v))

            if drifts:
                if allowed:
                    if not reason:
                        findings.append(("FehlenderGrund", key, table, reason, ""))
                else:
                    for kind, a, b in drifts:
                        findings.append((kind, key, table, a, b))
            elif allowed:
                findings.append(("VeralteterEintrag", key, table, reason, ""))

        # Eintraege, die es in keiner der beiden Fassungen gibt: ein Tippfehler
        # in der ID deckt sonst nichts ab, ohne dass es jemand merkt.
        for key in sorted(allowlist.get(table, {})):
            if key not in common:
                findings.append(("UnbekannterEintrag", key, table,
                                 allowlist[table][key], ""))

    return findings, summary


def _print_results(findings, summary):
    for table in TABLES:
        print("%s: %d gemeinsame Eintraege" % (table, summary[table]))

    if findings:
        print("")
        for kind, key, table, a, b in findings:
            if kind == "FehlenderGrund":
                print("%s %s: Erlaubnis ohne Grund" % (table, key))
            elif kind == "VeralteterEintrag":
                print("%s %s: veralteter Eintrag in der Erlaubnisliste "
                      "(keine Abweichung mehr)" % (table, key))
            elif kind == "UnbekannterEintrag":
                print("%s %s: Eintrag in der Erlaubnisliste, den es in beiden "
                      "Fassungen nicht gibt" % (table, key))
            else:
                print("%s %s: Abweichung (%s)" % (table, key, kind))
                print("  PC:  %s" % _trunc(a))
                print("  C64: %s" % _trunc(b))
        return 1
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        prog="content_drift",
        description="Vergleicht Spielinhalte zwischen PC und C64. Meldet "
                    "Abweichungen bei Dialogen, Notizen, NPC-Namen, "
                    "Item-Namen und Stimmungsbezeichnungen.")
    parser.add_argument("--allowlist",
                        metavar="PFAD",
                        help="Pfad zur Erlaubnisliste (Standard: %s)" % ALLOWLIST)
    args = parser.parse_args(argv)

    data = load_data()
    allowlist = load_allowlist(args.allowlist)
    findings, summary = compare(data, allowlist)
    return _print_results(findings, summary)


if __name__ == "__main__":
    sys.exit(main())
