# Plan: ein gemeinsamer Regelkern für PC und C64

Herkunft: Architekturreview (Kandidat 1). Die Regeln in `src/game.c` (757 Zeilen)
und `c64/src/game.c` (810 Zeilen) sind doppelt geschrieben und doppelt getestet;
`tools/content_drift.py` existiert nur, weil die Kopien auseinanderlaufen.
Der eigentliche `IMPLEMENTATION_PLAN.md` im Wurzelverzeichnis gehört dem POC
und bleibt unberührt.

**Leitplanke:** Jede Stufe baut und besteht die Tests auf beiden Plattformen und
bleibt im Speicherbudget der C64-Fassung (`make size` vor und nach jeder Stufe).

## Stufe 1: Spike `talk()`
**Goal**: Eine Regel liegt in `shared/talk.h`, beide Builds binden sie ein.
**Success Criteria**: PC-`ctest` und C64-`make test` grün; Mehrkosten der .prg
gemessen; entschieden, ob die Naht trägt.
**Tests**: bestehende Suiten (`tests/test_game.c`, `c64/tests/test_game.c`).
**Status**: Complete (Ergebnis unten, auf Branch `shared-rules-core-spike`)

Ergebnis:
- Beide Suiten grün, PC mit `-Werror -Wpedantic` ohne Warnung.
- C64: Code +12 Bytes, Nur-Lese-Daten +30 Bytes (das `phase`-Feld je Regel), also
  **+42 Bytes** insgesamt (13 400 → 13 412, 14 809 → 14 839).
- Was sich als Hindernis zeigte: `DialogueRule` hat auf beiden Seiten ein anderes
  Layout (PC: Enums und `phase`; C64: `uint8_t`, kein `phase`). Eine geteilte
  Regel braucht darum zuerst ein geteiltes Layout, also Stufe 2.
- Was geteilt blieb: die Auswahl der Regel. Die Wirkung (`inventory_add` gegen
  `bag[]++`, `emit`) bleibt bei der Plattform; das ist der künftige Adapter.

## Stufe 2: eine Inhaltsquelle
**Goal**: PC und C64 lesen dieselben Texte und Tabellen; Abweichungen sind
Einträge, die nur eine Seite hat, keine Kopien.
**Success Criteria**: C64-Größe innerhalb des Budgets; `content_drift.py` meldet
für Geteiltes nichts mehr.
**Tests**: `tools/test_content_drift.py`, beide Spielsuiten.
**Status**: In Progress (Branch `shared-content-source`)

- 2a, Texte: Complete. `shared/dialogues.inc` (74 Dialoge) und `shared/notes.inc`
  (30 Notizen) stehen in beiden `content.c` per `#include` mitten in der Tabelle.
  Nur `D_ENC_OFFER_BOWL` weicht ab und bleibt je Seite. Kein Generator: die
  Designated Initializer machen die Reihenfolge egal, und beide Enums behalten
  ihre eigene Liste. C64-Größe unverändiert (13 412 / 14 839), alle Tests grün.
  `content_drift.py` setzt die `.inc`-Dateien beim Lesen wieder ein.
- 2b, Regeltabellen (`DialogueRule`, `ExaminePoint`, `TileOverride`, `Place`):
  Not Started. Hier fehlt noch ein gemeinsames Layout und die Frage, ob ein
  Generator nötig wird.

## Stufe 3: gemeinsamer Zustand und Ausgabekanal
**Goal**: Ein Zustandslayout (flach, klein typisiert) im Kern; Ereignisse (PC)
und SFX (C64) sind zwei Adapter auf demselben Kanal.
**Success Criteria**: `game_action` im Kern; PC-Zusätze (Debug, Ereignislog)
liegen außen.
**Tests**: Kernsuite auf dem Host.
**Status**: Not Started

## Stufe 4: Regeln einzeln umziehen
**Goal**: `examine`, `mend`, `stake`, `encounter`, `push`/`move` wandern nacheinander
in den Kern. Kampf: `combat.c` (PC) und `fight_round` (C64) auseinandersetzen.
**Success Criteria**: je Schritt beide Plattformen grün und im Budget.
**Tests**: eine Kernsuite; die Plattformsuiten schrumpfen auf Adapter-Verhalten.
**Status**: Not Started
