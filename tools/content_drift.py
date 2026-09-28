#!/usr/bin/env python3
"""Vergleicht Spielinhalte zwischen der PC- und der C64-Umsetzung.

Zwei Teile:

Texte -- liest beide content.c-Dateien und die PC-Items aus inventory.c,
normalisiert und meldet Abweichungen bei Dialogen, Notizen, NPC-Namen,
Item-Namen und Stimmungsbezeichnungen.

Regeln -- liest dazu die Header und vergleicht Feld fuer Feld, was das
Spiel ausmacht: Gespraechsregeln, Untersuchungspunkte, Kachel-
Ueberschreibungen und Orte. Gleiche Texte sagen noch nicht, dass beide
Fassungen dasselbe Spiel spielen; wer wann was sagt, steht in diesen
Tabellen. Der PC schreibt sie mit benannten Initialisierern, der C64
positional, und die Zahlen hinter den Namen sind auf beiden Seiten
verschieden -- verglichen wird darum ueber die Namen, und die Feldfolge
kommt aus der Struktur im Header.

Beides gilt nur fuer Eintraege, die es in beiden Fassungen gibt: was dem
kleineren C64-Ausschnitt fehlt, ist kein Befund. Abweichungen sind nur
mit Eintrag in der Erlaubnisliste erlaubt; das Skript erzwingt eine
Entscheidung.
"""
import argparse
import json
import os
import re
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PC_CONTENT = os.path.join(REPO_ROOT, "src", "content.c")
PC_ITEMS = os.path.join(REPO_ROOT, "src", "inventory.c")
PC_HEADERS = (os.path.join(REPO_ROOT, "src", "content.h"),
              os.path.join(REPO_ROOT, "src", "inventory.h"),
              os.path.join(REPO_ROOT, "src", "world.h"))
C64_CONTENT = os.path.join(REPO_ROOT, "c64", "src", "content.c")
C64_HEADERS = (os.path.join(REPO_ROOT, "c64", "src", "content.h"),
               os.path.join(REPO_ROOT, "c64", "src", "world.h"))
ALLOWLIST = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                         "content_drift_allowlist.json")

TABLES = ("dialogues", "notes", "npcs", "items", "mood_names")

# Die Tabellen, die die Regeln ausmachen. Texte allein sagen nichts darueber,
# ob beide Fassungen dasselbe Spiel spielen: wer wann was sagt, steht hier.
RULE_TABLES = ("dialogue_rules", "examine_points", "tile_overrides", "places")
# Welche Struktur die Felder einer Regeltabelle benennt (steht im Header).
RULE_STRUCTS = {
    "dialogue_rules": "DialogueRule",
    "examine_points": "ExaminePoint",
    "tile_overrides": "TileOverride",
    "places": "Place",
}
# Woran ein Eintrag wiedererkannt wird. Kommt derselbe Schluessel mehrfach vor,
# haengt die Reihenfolge ein "#2", "#3" an -- die Reihenfolge entscheidet, denn
# die erste passende Regel gewinnt.
# Aus welchen Arrays eine Tabelle zusammenkommt. Der PC haelt die
# Ausgangs-Aenderungen getrennt und sieht sie in game_tile() zuerst an; der
# C64 hat eine Liste, in der sie oben stehen. Aneinandergehaengt in dieser
# Reihenfolge sind beide dasselbe.
RULE_ARRAYS = {
    "tile_overrides": {"pc": ("outcome_changes", "tile_overrides"),
                       "c64": ("tile_overrides",)},
}
RULE_KEYS = {
    "dialogue_rules": ("dialogue",),
    "examine_points": ("dialogue",),
    # Die Bedingung ist der Schluessel, die Kachel die Antwort darauf: am
    # selben Platz koennen mehrere Ueberschreibungen liegen (das Regal zeigt
    # trocknend oder fertig), und eine andere Kachel soll auffallen statt als
    # neuer Eintrag durchzugehen. Driftet umgekehrt die Bedingung selbst,
    # findet der Eintrag drueben keinen Partner und faellt heraus.
    "tile_overrides": ("map", "x", "y", "needs", "outcome"),
    "places": ("name",),
}
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


# --- Regeltabellen: Struktur aus dem Header, Eintraege aus der Quelle ---

def _strip_comments(text):
    return re.sub(r"/\*.*?\*/", " ", text, flags=re.DOTALL)


def _decl_names(decl):
    """Namen einer Feld-Deklaration: 'int map, x, y' -> [map, x, y]."""
    names = []
    for part in decl.split(","):
        # Der Typ steht nur vorn; in jedem Teil ist der Name der letzte
        # Bezeichner. Der Stern gehoert zum Typ, nicht zum Namen.
        ids = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", part.replace("*", " "))
        if ids:
            names.append(ids[-1])
    return names


def struct_fields(header, name):
    """Feldnamen einer Struktur, in Reihenfolge der Deklaration."""
    m = re.search(r"struct\s*\{([^{}]*)\}\s*" + re.escape(name) + r"\s*;",
                  header)
    if not m:
        return []
    fields = []
    for decl in _strip_comments(m.group(1)).split(";"):
        decl = decl.strip()
        if decl:
            fields.extend(_decl_names(decl))
    return fields


def enum_values(header):
    """Name -> Zahl fuer alle Aufzaehlungen einer Fassung.

    Gebraucht wird davon nur, welche Namen fuer null stehen: ein im Quelltext
    ausgelassenes Feld ist null, die andere Fassung schreibt dort vielleicht
    ITEM_NONE oder MAP_VILLAGE."""
    values = {}
    bodies = re.findall(
        r"enum\s*\{([^{}]*)\}\s*[A-Za-z_][A-Za-z0-9_]*?\s*;", header)
    bodies += re.findall(r"enum\s*\{([^{}]*)\}\s*;", header)
    for body in bodies:
        number = 0
        for part in _strip_comments(body).split(","):
            part = part.strip()
            if not part:
                continue
            if "=" in part:
                name, expr = [t.strip() for t in part.split("=", 1)]
                try:
                    number = int(expr, 0)
                except ValueError:
                    number = None
            else:
                name = part
            if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", name):
                values[name] = number
            number = number + 1 if number is not None else None
    return values


def _macro_defs(text):
    """Funktionsartige Makros derselben Datei: Name -> (Parameter, Rumpf)."""
    joined = re.sub(r"\\\n", " ", text)
    defs = {}
    pattern = r"^#define\s+([A-Za-z_][A-Za-z0-9_]*)\(([^)]*)\)\s+(.+)$"
    for m in re.finditer(pattern, joined, re.M):
        params = [t.strip() for t in m.group(2).split(",") if t.strip()]
        defs[m.group(1)] = (params, m.group(3).strip())
    return defs


def _split_top_level(text, separator=","):
    """Teilt an Kommas ausserhalb von Klammern und Anfuehrungszeichen."""
    parts, depth, current, quote = [], 0, "", None
    for c in text:
        if quote:
            current += c
            if c == quote:
                quote = None
            continue
        if c in "\"'":
            quote = c
            current += c
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == separator and depth == 0:
            parts.append(current.strip())
            current = ""
        else:
            current += c
    if current.strip():
        parts.append(current.strip())
    return parts


def expand_macros(body, defs):
    """Setzt MARK(14, 14) und dergleichen ein: sonst waere der ganze
    Eintrag ein einziges Feld."""
    for name, (params, template) in defs.items():
        while True:
            m = re.search(r"\b" + re.escape(name) + r"\s*\(", body)
            if not m:
                break
            depth, i = 0, m.end() - 1
            while i < len(body):
                if body[i] == "(":
                    depth += 1
                elif body[i] == ")":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            args = _split_top_level(body[m.end():i])
            filled = template
            for param, arg in zip(params, args):
                filled = re.sub(r"\b" + re.escape(param) + r"\b", arg, filled)
            body = body[:m.start()] + filled + body[i + 1:]
    return body


def rule_arrays(table, side):
    """Die Arrays, aus denen eine Regeltabelle einer Fassung besteht."""
    return RULE_ARRAYS.get(table, {}).get(side, (table,))


def parse_rule_table(source, header, table, side="pc"):
    """Liste von (Schluessel, Feld -> Text) fuer eine Regeltabelle.

    Ein nicht gesetztes Feld steht als None da: in C ist es null, und die
    andere Fassung schreibt dort vielleicht einen Namen, der null bedeutet."""
    fields = struct_fields(header, RULE_STRUCTS[table])
    macros = _macro_defs(source)
    groups = []
    for name in rule_arrays(table, side):
        if not re.search(r"\b" + re.escape(name) + r"\s*\[", source):
            continue
        body = expand_macros(_extract_array(source, name), macros)
        groups.extend(g for _, g in _brace_groups(_strip_comments(body)))
    entries, seen = [], {}
    for group in groups:
        values = {}
        position = 0
        for value in _split_top_level(group):
            m = re.match(r"^\.\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*)$", value,
                         re.DOTALL)
            if m:
                values[m.group(1)] = " ".join(m.group(2).split())
            else:
                if position < len(fields):
                    values[fields[position]] = " ".join(value.split())
                position += 1
        key = ":".join(str(values.get(f)) for f in RULE_KEYS[table])
        seen[key] = seen.get(key, 0) + 1
        if seen[key] > 1:
            key = "%s#%d" % (key, seen[key])
        entries.append((key, values))
    return entries


def _is_zero(value, values):
    if value is None:
        return True
    return value == "0" or values.get(value) == 0


def _fields_differ(pc_value, c64_value, pc_values, c64_values):
    if _is_zero(pc_value, pc_values) and _is_zero(c64_value, c64_values):
        return False
    if pc_value is None or c64_value is None:
        return True
    return pc_value != c64_value


def parse_texts(pc_content_text, pc_items_text, c64_content_text,
                pc_header_text="", c64_header_text=""):
    """Parst Quelltexte und gibt dict mit 'pc' und 'c64' zurueck.

    PC: dialogues, notes, npcs, mood_names aus pc_content_text;
        items aus pc_items_text.
    C64: alle Tabellen aus c64_content_text (items dort ebenfalls).
    Die Regeltabellen kommen dazu, sobald die Header mitgegeben werden:
    aus ihnen kommen die Feldnamen und die Namen, die fuer null stehen."""
    data = {"pc": {}, "c64": {}}
    for name in TABLES:
        pc_src = pc_items_text if name == "items" else pc_content_text
        data["pc"][name] = _parse_table(pc_src, name)
        data["c64"][name] = _parse_table(c64_content_text, name)
    for side, source, header in (("pc", pc_content_text, pc_header_text),
                                 ("c64", c64_content_text, c64_header_text)):
        data[side]["rules"] = {}
        data[side]["values"] = enum_values(header) if header else {}
        for table in RULE_TABLES:
            if not header or not any(
                    re.search(r"\b" + re.escape(name) + r"\s*\[", source)
                    for name in rule_arrays(table, side)):
                continue
            data[side]["rules"][table] = parse_rule_table(
                source, header, table, side)
    return data


def load_data():
    pc_header = "\n".join(_read(f) for f in PC_HEADERS)
    c64_header = "\n".join(_read(f) for f in C64_HEADERS)
    return parse_texts(_read(PC_CONTENT), _read(PC_ITEMS), _read(C64_CONTENT),
                       pc_header, c64_header)


def load_allowlist(path=None):
    if path is None:
        path = ALLOWLIST
    if not os.path.exists(path):
        return {t: {} for t in TABLES + RULE_TABLES}
    with open(path, encoding="utf-8") as f:
        raw = json.load(f)
    result = {}
    for table in TABLES + RULE_TABLES:
        entries = raw.get(table, {})
        result[table] = {}
        for key, reason in entries.items():
            result[table][key] = reason if isinstance(reason, str) else ""
    return result


def _trunc(s, n=100):
    s = str(s).replace("\n", " ")
    return s if len(s) <= n else s[:n] + "..."


def vergleiche(pc_tables, c64_tables, ausnahmen=None,
               pc_header="", c64_header=""):
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
    for side, tables, header in (("pc", pc_tables, pc_header),
                                 ("c64", c64_tables, c64_header)):
        data[side]["rules"] = {}
        data[side]["values"] = enum_values(header) if header else {}
        for table in RULE_TABLES:
            source = tables.get(table, "")
            if source and header:
                data[side]["rules"][table] = parse_rule_table(
                    source, header, table, side)
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

    findings.extend(_compare_rules(data, allowlist, summary))
    return findings, summary


def _compare_rules(data, allowlist, summary):
    """Vergleicht die Regeltabellen Feld fuer Feld.

    Verglichen wird nur, was beide Fassungen kennen: ein Eintrag, den es
    drueben nicht gibt, gehoert zum bewusst kleineren Ausschnitt des C64 und
    steht in dessen README. Felder, die nur eine Struktur hat (der PC kennt
    Phase und Tag), bleiben ebenfalls aussen vor. Ein im Quelltext
    ausgelassenes Feld ist null und gleicht jedem Namen, der null bedeutet."""
    findings = []
    for table in RULE_TABLES:
        pc_rules = data["pc"].get("rules", {})
        c64_rules = data["c64"].get("rules", {})
        if table not in pc_rules and table not in c64_rules:
            # Gar nicht eingelesen (Aufruf ohne Header): nichts zu sagen.
            continue
        pc_entries = dict(pc_rules.get(table, []))
        c64_entries = dict(c64_rules.get(table, []))
        common = sorted(set(pc_entries) & set(c64_entries))
        summary[table] = len(common)
        exceptions = allowlist.get(table, {})
        hit = set()
        for key in common:
            pc_fields = pc_entries[key]
            c64_fields = c64_entries[key]
            for field in sorted(set(pc_fields) & set(c64_fields)):
                if not _fields_differ(
                        pc_fields.get(field), c64_fields.get(field),
                        data["pc"]["values"], data["c64"]["values"]):
                    continue
                where = "%s|%s" % (key, field)
                hit.add(where)
                if where in exceptions:
                    if not exceptions[where]:
                        findings.append(
                            ("FehlenderGrund", where, table, "", ""))
                else:
                    findings.append(("Feld", where, table,
                                     str(pc_fields.get(field)),
                                     str(c64_fields.get(field))))
        findings.extend(
            _unreachable_texts(data, table, pc_entries, c64_entries))
        for where in sorted(exceptions):
            if where in hit:
                continue
            key = where.split("|")[0]
            kind = ("VeralteterEintrag" if key in common
                    else "UnbekannterEintrag")
            findings.append((kind, where, table, exceptions[where], ""))
    return findings


def _unreachable_texts(data, table, pc_entries, c64_entries):
    """Text in beiden Fassungen, aber nur in einer eine Regel, die ihn zeigt.

    Faengt den Fall, den der Schluessel sonst verschluckt: wandert die
    Dialog-ID einer Regel, findet der Eintrag drueben keinen Partner und
    fiele stillschweigend heraus. Steht der Text auf beiden Seiten, ist eine
    fehlende Regel aber kein kleinerer Ausschnitt, sondern ein Text, den
    niemand mehr zu sehen bekommt."""
    if table not in ("dialogue_rules", "examine_points"):
        return []
    both = (set(data["pc"].get("dialogues", {}))
            & set(data["c64"].get("dialogues", {})))
    if not both:
        return []
    findings = []
    pc_shown = {key.split("#")[0] for key in pc_entries}
    c64_shown = {key.split("#")[0] for key in c64_entries}
    for key in sorted((pc_shown ^ c64_shown) & both):
        fehlt = "C64" if key in pc_shown else "PC"
        findings.append(("OhneRegel", key, table, fehlt, ""))
    return findings


def _print_results(findings, summary):
    for table in TABLES + RULE_TABLES:
        print("%s: %d gemeinsame Eintraege" % (table, summary[table]))

    if findings:
        print("")
        for kind, key, table, a, b in findings:
            if kind == "FehlenderGrund":
                print("%s %s: Erlaubnis ohne Grund" % (table, key))
            elif kind == "VeralteterEintrag":
                print("%s %s: veralteter Eintrag in der Erlaubnisliste "
                      "(keine Abweichung mehr)" % (table, key))
            elif kind == "OhneRegel":
                print("%s %s: den Text gibt es in beiden Fassungen, aber "
                      "%s hat keine Regel, die ihn zeigt" % (table, key, a))
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
                    "Item-Namen, Stimmungsbezeichnungen und den Regeln: "
                    "Gespraechsregeln, Untersuchungspunkte, Kachel-"
                    "Ueberschreibungen und Orte.")
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
