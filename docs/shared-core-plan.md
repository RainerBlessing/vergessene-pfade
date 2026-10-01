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
**Status**: Complete (Branch `shared-content-source`; `TileOverride` bleibt je Seite)

- 2a, Texte: Complete. `shared/dialogues.inc` (74 Dialoge) und `shared/notes.inc`
  (30 Notizen) stehen in beiden `content.c` per `#include` mitten in der Tabelle.
  Nur `D_ENC_OFFER_BOWL` weicht ab und bleibt je Seite. Kein Generator: die
  Designated Initializer machen die Reihenfolge egal, und beide Enums behalten
  ihre eigene Liste. C64-Größe unverändiert (13 412 / 14 839), alle Tests grün.
  `content_drift.py` setzt die `.inc`-Dateien beim Lesen wieder ein.
- 2b, Regeltabellen: Complete.
  - `Place`: Complete. `shared/places.inc`, beide Tabellen identisch.
  - `DialogueRule`: Complete. 30 Regeln stehen in `shared/dialogue_rules_a.inc`
    (erste Regel) und `_b.inc` (der Rest); die 10 PC-eigenen Morgenregeln liegen
    dazwischen und stehen nur in `src/content.c`. Die drei Regeln, die auf dem PC
    in die Nacht und auf dem C64 zur Schlusstafel führen, nennen
    `OPEN_AFTER_OUTCOME`; jede Seite definiert das Makro. Der Driftmelder löst
    solche Aliase auf, damit die Erlaubnisliste weiter gilt.
  - `ExaminePoint`: Complete. 30 Punkte in `shared/examine_points_a.inc` und
    `_b.inc`; dazwischen steht auf dem PC `D_X_INSCRIPTION_LATE`, das vor der
    geteilten Inschrift greifen muss. Die übrigen PC-Punkte (Fuchsbau, Futon,
    graue Spur) folgen am Ende. Die Reihenfolge ist nur innerhalb gleicher Kachel
    oder Position wichtig; die geteilten Punkte stehen jetzt in der Reihenfolge
    des C64. Eine echte Abweichung kam dabei ans Licht: `point_at` prüft auf dem
    C64 die Karte auch bei `POINT_STONE`, auf dem PC nicht. Die drei Stein-Punkte
    tragen darum `.map = MAP_FOREST`; auf dem PC ändert das nichts.
  - `TileOverride`: bleibt je Seite (PC hat `phase` und `tag`, zwei Tabellen).

## Stufe 3: gemeinsamer Zustand und Ausgabekanal
**Goal**: Ein Zustandslayout (flach, klein typisiert) im Kern; Ereignisse (PC)
und SFX (C64) sind zwei Adapter auf demselben Kanal.
**Success Criteria**: `game_action` im Kern; PC-Zusätze (Debug, Ereignislog)
liegen außen.
**Tests**: Kernsuite auf dem Host.
**Status**: Complete (Branch `shared-core-state`)

- 3a, Ausgabekanal: Complete. `shared/feedback.h` hält `EventType`, `SfxId` und
  `sfx_for_event()`. Die PC-Zuordnung Ereignis → Klang (`audio_events`) und acht
  `cue()`-Stellen der C64-Fassung riefen dieselbe Entscheidung an verschiedenen
  Orten auf; jetzt steht sie einmal da. Die C64 hat dafür `emit()`, das nur den
  Klang merkt. Kosten: +22 Bytes Code, Tests beider Seiten grün (PC mit SDL
  gebaut, 4 von 4).
  Bewusst nicht geteilt, weil die Fassungen es verschieden auslösen:
  - `SFX_WRITE`: PC bei jeder neuen Beobachtung (`EV_OBSERVE`), C64 bei jeder
    neuen Notiz. Der Kommentar sagt „in das Notizbuch“, die C64-Fassung folgt
    ihm. Offen: auf eine Notiz-Regel einigen.
  - Klick und Schritt: PC im Frontend (`main.c`, Zähler `steps`), C64 in der Aktion.
- 3b, Zustandslayout: Complete. `shared/state.h` hat `GAME_CORE_FIELDS`, die
  Felder, die beide `Game` gleich führen (Position, Sprecher, Seite, Auswahl,
  Beobachtungen, Notizen, Stein, Schale, Daigo, Meldung); jede Fassung gibt
  `Coord` und `Count` vor (PC: `int`, C64: `int8_t`/`uint8_t`), damit die PC-Fassung
  nicht schmaler wird und die C64-Fassung nicht breiter. Enum-Felder (`state`,
  `mood`, `outcome`, `opens`) bleiben je Seite unter gleichem Namen. C64-Größe
  und BSS unverändert; PC mit SDL, ASan/UBSan und clang-tidy sauber.
- 3c, Kampf: Complete, aber kleiner als geplant. `shared/fight.h` hält die Zahlen
  (`PLAYER_*`, `KAMI_*`) und den Wortlaut der Kampfmeldung; die PC-Fassung nimmt
  den Text der C64-Fassung („Das Kraut lindert deine Wunden“), und bei einer
  Niederlage steht dort kein Satz mehr (das sagt die Szene danach). Die
  Rechnung selbst (Schaden, Würfel) bleibt je Fassung: Ein geteiltes
  `fight_blows()` in `shared/fight.c` kostete die C64-Fassung **+371 Bytes**
  (mit der geteilten Meldung +590), weil die Ergebnisse über Zeiger zurückkommen und
  der 6502 über Zeiger teuer zugreift. Das ist gemessen, nicht geschätzt (Code
  13 434 → 13 805 beziehungsweise 14 024). Beide Seiten würfeln gleich
  (`random * 1664525 + 1013904223`), unterscheiden sich aber noch darin, dass
  `combat_begin()` auf dem PC den Würfel auf 42 zurücksetzt und die C64-Fassung
  ihn weiterlaufen lässt.

## Platzmessung vor Stufe 4 (C64)

Gemessen mit `make size` am 2026-10-01.

| | Datei | RAM belegt |
|---|---|---|
| Basis (`pc-stein-rueckmeldung`) | 28 252 | 28 647 |
| nach Stufe 1–3 | 28 314 | 28 709 |
| **Mehrkosten Stufe 1–3** | **+62** | **+62** |

Die +62 Bytes sind das Feld `phase` je Regel (+42, Stufe 1) und `emit()` (+22,
Stufe 3a), abzüglich 2.

**Was frei ist.** Das Linkerskript (`mos-platform/c64/lib/link.ld`) gibt dem Programm
`$0801` bis `$CFFF`, also 51 199 Bytes; der weiche Stapel wächst von `$D000` nach
unten. Bei 28 709 belegten Bytes bleiben **rund 22 500 Bytes (44 %)**. Die
README rechnet „43 % von 64K“; der nutzbare Teil ist kleiner, aber die Reserve ist
trotzdem groß. Der Nullseiten-Anteil (`-mlto-zp=110`) ist ausgeschöpft: Mehr Code
bekommt dort keine Plätze mehr und wird dadurch langsamer, nicht ungültig.

**Was knapp werden kann.** Inhalt wächst (Issue #25: Texte und Karten). Die
Nur-Lese-Daten sind heute 14 839 Bytes; verdoppeln sie sich, bleiben etwa 7 700
Bytes für Code und Stapel. Ein Kern, der ein paar hundert Bytes kostet, ist
bezahlbar, gehört aber gegen Inhalt abgewogen.

**Was die Messungen über die Form des geteilten Codes sagen.**
- Teurer Code ist der, der Ergebnisse über Zeiger zurückgibt oder Zustand
  hin- und herkopiert: `fight_blows()` +371 Bytes, mit geteilter Meldung +590.
- Billig ist Code, der wie auf der C64 direkt auf `g->feld` zugreift
  (Stufe 1: Auswahl der Regel, +12 Bytes Code; Stufe 3a: +22).
- Folge für Stufe 4: Regeln als `static inline`-Funktionen in einer `.inc`, die
  `game.c` beider Fassungen einbindet (wie `shared/talk.h`), mit der
  C64-Form als Vorlage; die PC-Fassung passt sich an, nicht umgekehrt.
- Nicht gemessen: Laufzeit (`make bench` braucht VICE mit Anzeige). Die Regeln
  laufen einmal je Tastendruck; ein Bildaufbau kostet rund 199 000 Takte.

## Stufe 4: Regeln einzeln umziehen
**Goal**: `examine`, `mend`, `stake`, `encounter`, `push`/`move` wandern nacheinander
in den Kern. Kampf: die Rechnung bleibt je Fassung (siehe 3c).
**Success Criteria**: je Schritt beide Plattformen grün und im Budget.
**Tests**: eine Kernsuite; die Plattformsuiten schrumpfen auf Adapter-Verhalten.
**Status**: In Progress (Branch `shared-rules-mend`)

- 4a, `mend_action`: Complete (Spike). `shared/mend.h` steht in beiden `game.c`.
  **C64-Größe unverändert** (Datei 28 314, Code 13 432), beide Suiten und PC
  (SDL, ASan/UBSan) grün. Das bestätigt die Form aus der Platzmessung: eine
  `static`-Funktion in einer Headerdatei, die direkt auf `g->feld` zugreift.
  Was dafür auf jeder Seite in kleiner Form da sein muss: `select_move`, `show`,
  `take_item`, `learn`, `open_scene`, `emit`. `Count` als Feldtyp für die
  Stückliste lässt `game_mend_pieces` auf beiden Seiten unverändert. Zwei
  Unterschiede, die dabei verschwanden: Die PC-Fassung loggt auch ein falsches
  Teilstück (`EV_MEND` mit `b = 0`), die C64-Fassung ließ den Aufruf aus; jetzt
  ruft beide ihn auf, und `sfx_for_event` macht daraus auf der C64 keinen Klang.
- 4b, `stake_here`: Complete. `shared/stake.h`. C64-Größe **+9 Bytes** (Datei
  28 314 → 28 323); beide Suiten und PC (SDL, ASan/UBSan) grün. Die erste
  Fassung kostete +60 Bytes, weil `stakes[i].x != g->x` auf der C64 als
  16-Bit-Vergleich übersetzt wurde. Die C64-Vorlage verglich Bytes
  (`(uint8_t)g->x`); mit demselben Cast im geteilten Code sind es +9. Lehre
  für die weiteren Regeln: Die **Typen und Casts der C64-Fassung** gehören
  mit in den geteilten Code, nicht nur die Anweisungsfolge. Die C64-Fassung
  hat dafür `finish()` bekommen (setzt den Ausgang einmal und meldet ihn); der
  Aufruf von `emit` darin kostet nichts messbar, weil der Übersetzer ihn
  zusammenfaltet.
- 4c, Stein: Complete. `shared/stone.h` hat `game_can_push`, `stone_at` und
  `push_stone`; jede Fassung liefert `stone_fits()` (darf der Stein auf diese
  Kachel: PC über Felder, C64 über Flags). C64-Datei 28 323 → 28 264, also
  **−59 Bytes**; beide Suiten und PC (SDL, ASan/UBSan) grün. Warum es kleiner
  wurde, habe ich nicht untersucht (vermutlich andere Einbettung unter LTO);
  gemessen, nicht begründet.
  `reset_stone`: zuerst bewusst nicht geteilt, dann entschieden und geteilt
  (siehe unten).
- 4d, `reset_stone`: Complete. Das Zurückrollen des Steins ist zu hören (die C64
  meldet jetzt `EV_STONE_PUSH` wie der PC, +12 Bytes, Test
  `a_stuck_stone_rolls_back`). Die C64 setzte dabei Daigo auf seinen Platz
  zurück, der PC nicht; geprüft: Das beendet den Begleit-Zustand nicht
  (`daigo_follows` blieb unberührt, der nächste Schritt überschreibt die
  Position), und der Test bleibt ohne die Zeilen grün. Entschieden: Daigo wird
  nicht zurückgesetzt, das Verhalten des PC gilt. Damit steht `reset_stone`
  in `shared/stone.h`. C64-Datei 28 266 (−10 gegenüber der Fassung mit
  Daigo-Zeilen); beide Suiten und PC (SDL, ASan/UBSan) grün.
- 4e, Untersuchen: Complete. `shared/examine.h` hat `point_at`, `item_point_at`,
  `point_for_item`, `examine` und die Tabelle der Nachbarn; jede Fassung
  behält `use_point` und `examine_nothing` (der PC protokolliert, hat das Morgen-
  Ende und die Phase). **Entschieden:** Der PC prüft wie die C64 (Issue #17)
  auch die Nachbarfelder, beim Untersuchen und beim Benutzen eines Gegenstands.
  Neue PC-Tests `test_examine_reaches_neighbours` und
  `test_item_reaches_neighbour` (zuerst rot). C64-Datei **unverändert** 28 266;
  beide Suiten und PC (SDL, ASan/UBSan, clang-tidy) grün.
- 4f, Begegnung: Complete. `shared/encounter.h` hat `offer_at_hand`,
  `game_encounter_options`, `step_back`, `begin_encounter` und `encounter_action`;
  je Fassung bleiben `carries`, `encounter_say` (PC: Text in die Meldung, C64:
  Zeile der Dialogtabelle), `fight_round` und `plain_tile` (früher `stone_fits`,
  jetzt auch für den Schritt zurück). `game_encounter_options` hat auf beiden
  Seiten dieselbe Form (`Count`). Das Ereignis `EV_ENCOUNTER_ACTION` meldet jetzt
  jede Wahl, nicht nur den Schlag; auf der C64 macht daraus nur `ENC_ATTACK`
  einen Klang (Test `only_a_blow_is_heard`, mit Mutation geprüft). Die
  Reihenfolge lernen → Zeile → melden bleibt die des PC, damit das Protokoll
  gleich bleibt. **C64-Datei sauber gebaut 28 266 → 28 246 (−20).**
- Makefile der C64: Die Dateien in `shared/` waren keine Abhängigkeit. Eine
  Änderung nur dort baute nichts neu, und `make test` lief gegen eine alte
  Binärdatei. Beim Prüfen der Begegnung fiel das auf (ein Mutationstest scheiterte
  nicht). Behoben (`SHARED` im Makefile). Die Größen je Branch sind sauber
  neu gebaut: Basis 28 252, Stufe 3 28 314, Stufe 4 (#51) 28 266.
