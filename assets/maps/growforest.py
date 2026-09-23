#!/usr/bin/env python3
"""Setzt die Kiefern im Wald neu: Gruppen und Lichtungen statt Diagonalgitter.

Angefasst wird ausschliesslich `T` (Kiefer) und `.` (Wiese). Alles, was die
Geschichte traegt -- Hain, Grenzsteine, Mulde, Schleifspur, Schrein, Kies, Bach,
Lager, Fuchsbau, Wege, Uebergaenge -- bleibt Feld fuer Feld, wo es ist.

Das Ergebnis wird eingecheckt, nicht zur Laufzeit erzeugt: Der Seed steht hier,
die Karte ist damit reproduzierbar, im Diff lesbar und umkehrbar.

    python3 assets/maps/growforest.py [--seed N] [--dry-run]

Warum ueberhaupt: Das alte Gitter erzeugte 55 Sackgassen -- Felder mit nur einem
Ausgang. Wer dort hineinlaeuft, haelt es fuer verschluckte Tasten, nicht fuer
Wald (siehe Issue #18).
"""
import argparse
import pathlib
import random
from collections import deque

MAP = pathlib.Path(__file__).with_name("forest.map")
# Moosboden `h` ist begehbar -- er ist bewacht, nicht versperrt.
PASSABLE = set(".,_+<>*=smdtpuh")
SEED = 11
CLUMPS = 40        # Baumgruppen
SPREAD = 2.2       # wie weit eine Gruppe streut, in Feldern
FILL = 0.7         # wie dicht eine Gruppe steht
SCATTERED = 60     # Einzelbaeume zwischen den Gruppen
DEAD_END_LIMIT = 20

# Die Tierfaehrten stehen nicht in der Karte, sondern als Ueberschreibungen im
# Inhalt (TRACK(...) in content.c). Auf ihnen darf kein Baum stehen, sonst zeigt
# der Wald eine Spur, die niemand begehen kann.
TRACKS = [(8, 19), (9, 18), (10, 17), (11, 16), (12, 15), (14, 14), (16, 14),
          (18, 15), (20, 15), (22, 15), (26, 15), (28, 15), (30, 14), (32, 14),
          (34, 13)]
# Was erreichbar bleiben muss: Uebergang, Fuchsbau, Lager mit Auftragsbuch und
# Daigo, Schrein, Stein und Mulde, dazu jede Faehrte (die drei Pfahlstellen
# liegen auf ihnen).
INTEREST = [(24, 38), (5, 21), (12, 31), (11, 32), (38, 21), (23, 16), (24, 13),
            (24, 12)] + TRACKS
START = (24, 38)
# Kein Baum vor einer Sache: was untersucht oder betreten wird, bleibt von allen
# Seiten anlaufbar. Wiese, Kiefer, Fels und Moosboden sind keine Sachen.
PLAIN = set(".T^h")


def load():
    lines = MAP.read_text().splitlines()
    width, height = (int(n) for n in lines[0].split())
    rows = [list(r) for r in lines[1:1 + height]]
    assert len(rows) == height and all(len(r) == width for r in rows)
    return rows, width, height


def free(rows, x, y):
    return 0 <= y < len(rows) and 0 <= x < len(rows[0]) and rows[y][x] in PASSABLE


def outgoing(rows, x, y):
    return [(x + dx, y + dy) for dx, dy in ((0, -1), (1, 0), (0, 1), (-1, 0))
            if free(rows, x + dx, y + dy)]


def reachable(rows, start):
    seen, queue = {start}, deque([start])
    while queue:
        x, y = queue.popleft()
        for n in outgoing(rows, x, y):
            if n not in seen:
                seen.add(n)
                queue.append(n)
    return seen


def approaches(rows, width, height):
    """Felder, die neben einer Sache liegen -- dort waechst nichts Neues."""
    cells = set()
    for y in range(height):
        for x in range(width):
            if rows[y][x] in PLAIN:
                continue
            for dx, dy in ((0, -1), (1, 0), (0, 1), (-1, 0)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < width and 0 <= ny < height:
                    cells.add((nx, ny))
    return cells


def grow(rows, width, height, seed):
    rng = random.Random(seed)
    keep = [[rows[y][x] not in "T." for x in range(width)] for y in range(height)]
    for x, y in approaches(rows, width, height):
        keep[y][x] = True
    for x, y in TRACKS:
        keep[y][x] = True
    out = [["." if c == "T" else c for c in row] for row in rows]

    def plant(x, y):
        if 0 <= x < width and 0 <= y < height and not keep[y][x] and out[y][x] == ".":
            out[y][x] = "T"

    for _ in range(CLUMPS):
        cx, cy = rng.randrange(1, width - 1), rng.randrange(1, height - 1)
        for _ in range(rng.randint(6, 16)):
            if rng.random() <= FILL:
                plant(int(rng.gauss(cx, SPREAD)), int(rng.gauss(cy, SPREAD)))
    for _ in range(SCATTERED):
        if rng.random() < 0.5:
            plant(rng.randrange(width), rng.randrange(height))

    # Sackgassen aufloesen: wo nur ein Ausgang bleibt, faellt ein Baum daneben.
    for _ in range(3):
        for y in range(1, height - 1):
            for x in range(1, width - 1):
                if not free(out, x, y) or len(outgoing(out, x, y)) > 1:
                    continue
                for dx, dy in ((0, -1), (1, 0), (0, 1), (-1, 0)):
                    if out[y + dy][x + dx] == "T":
                        out[y + dy][x + dx] = "."
                        break
    return out


def approachable(rows):
    """Sachen, die man erreichen und ansehen kann -- Bach und Bambus jenseits
    des Wassers gehoeren nicht dazu, die waren nie anlaufbar."""
    seen = reachable(rows, START)
    out = set()
    for y in range(len(rows)):
        for x in range(len(rows[0])):
            if rows[y][x] in PLAIN:
                continue
            if any(n in seen for n in outgoing(rows, x, y)):
                out.add((x, y))
    return out


def dead_ends(rows):
    return [(x, y) for y in range(len(rows)) for x in range(len(rows[0]))
            if free(rows, x, y) and len(outgoing(rows, x, y)) == 1]


def check(rows, before):
    """Was die neue Karte gegenueber der alten verschlechtert."""
    problems = []
    seen = reachable(rows, START)
    for x, y in INTEREST:
        if free(rows, x, y) and (x, y) not in seen:
            problems.append(f"({x},{y}) ist abgeschnitten")
    lost = approachable(before) - approachable(rows)
    for x, y in sorted(lost):
        problems.append(f"({x},{y}) '{before[y][x]}' ist nicht mehr anlaufbar")
    dead, dead_before = len(dead_ends(rows)), len(dead_ends(before))
    if dead > dead_before:
        problems.append(f"{dead} Sackgassen statt {dead_before}")
    return problems, dead, sum(r.count("T") for r in rows)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--seed", type=int, default=SEED)
    p.add_argument("--dry-run", action="store_true")
    args = p.parse_args()

    rows, width, height = load()
    _, dead_before, trees_before = check(rows, rows)
    grown = grow(rows, width, height, args.seed)
    problems, dead, trees = check(grown, rows)
    print(f"vorher:  {trees_before} Kiefern, {dead_before} Sackgassen")
    print(f"nachher: {trees} Kiefern, {dead} Sackgassen (Seed {args.seed})")
    if problems:
        raise SystemExit("verworfen: " + "; ".join(problems))
    if args.dry_run:
        print("\n".join("".join(r) for r in grown))
        return
    MAP.write_text(f"{width} {height}\n" + "\n".join("".join(r) for r in grown) + "\n")
    print(f"geschrieben: {MAP}")


if __name__ == "__main__":
    main()
