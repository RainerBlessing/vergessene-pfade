# C64-POC „Die vergessenen Pfade“

Ziel: prüfen, ob der Loop **Erkunden → Beobachten → Verstehen → Handeln** in
40x25 PETSCII-Zeichen und auf einer 1-MHz-Maschine trägt. Vorlage für Aufbau,
Toolchain und VICE-Start ist `~/projekte/c64u/fps-monitor`.

Umfang (mit Rainer abgestimmt): Erkundung beider Karten, die Begegnung am
Hainrand und alle drei Ausgänge. Nacht im Gasthaus und Morgen-Phase
([#4](../../../issues/4)) kommen mit den Stufen 7 bis 10 unten dazu. Klang (#5) ist als kurze SID-Effekte in der Hauptschleife enthalten.

Alle Stufen hier sind fertig. Was als Nächstes ansteht, steht in den Issues,
nicht in diesem Plan: erst einmal durchspielen ([#7](../../../issues/7)) und auf
echter Hardware prüfen ([#6](../../../issues/6)); Ausbau danach.

Wie viel Speicher der Ausschnitt belegt und was daran schon verdichtet wurde,
sagt `make size` und der Abschnitt „Speicher“ im [README](README.md) — die
Grundlage für das Nachladen ([#11](../../../issues/11)).

Die Spiellogik ist dieselbe Tabellenlogik wie in der SDL-Fassung (`../src`),
nur auf diesen Umfang gekürzt. Texte sind wörtlich übernommen.

## Stage 1: Gerüst und Welt
**Ziel**: `build/vergessene-pfade.prg` startet, zeigt Titelseite und begehbare Karten.
**Erfolgskriterien**: Gehen, Kartenübergang Dorf↔Wald, Scroll-Viewport, keine
Kollisionsfehler an Rändern.
**Tests**: Host-Test `walk_between_maps`.
**Status**: Complete

## Stage 2: Beobachten
**Ziel**: Untersuchen, NPC-Dialoge nach Regeltabelle, Beobachtungen, Notizbuch, Tasche.
**Erfolgskriterien**: Fuchs versorgen schaltet Fährten frei; Schale + Hauszeichen
führen über Sumi zu `OBS_BOWL_OWNER`.
**Tests**: Host-Tests `fox_unlocks_tracks`, `bowl_story_needs_both_marks`.
**Status**: Complete

## Stage 3: Begegnung
**Ziel**: Kami am Hainrand mit Gemüt, Kampf, Ausgang `OUT_FIGHT` verändert die Welt.
**Erfolgskriterien**: Moosboden löst die Begegnung aus; Zurückweichen beendet sie;
Angreifen führt zum Kampf; nach dem Sieg stehen Holzstapel im Dorf und Stümpfe im Hain.
**Tests**: Host-Tests `retreat_ends_encounter`, `fight_changes_world`.
**Status**: Complete

## Stage 4: Bauen, Testen, Laufen
**Ziel**: `make`, `make test`, `./run-vice.sh`, README.
**Erfolgskriterien**: PRG passt in den Speicher, Host-Tests grün, Lauf in VICE geprüft.
**Status**: Complete

## Stage 5: Alte Grenze
**Ziel**: Den versetzten Stein zurück in die Mulde schieben, ein Feld je Schritt.
**Erfolgskriterien**: Schieben nur mit Schleifspur **und** Mulde beobachtet; ein
verklemmter Stein lässt sich durch Ansehen zurücksetzen; in der Mulde beruhigt sich
der Kami, der Hainrand bleibt, das Lager verliert Grund, Sumi und Daigo reagieren.
**Tests**: `pushing_needs_both_observations`, `restore_old_boundary`,
`a_stuck_stone_rolls_back`.
**Status**: Complete

## Stage 6: Kintsugi und Kompromiss
**Ziel**: Die Kette von den Scherben bis zur neuen Grenze: Zeichen, Sumis
Geschichte, Reparaturansicht, Trocknen, Darbringen, Daigo folgt, drei Pfähle.
**Erfolgskriterien**: Oriha flickt erst mit `OBS_BOWL_OWNER`; ein falsches Stück
kostet nichts; die Schale wird beim Rückweg aus dem Wald trocken; die geflickte
Schale macht den Kami ruhig; Daigo geht nur mit Spuren und Auftragsbuch mit;
drei Pfähle setzen `OUT_MEND` mit Totholz (Gewinn) und verlorenem Hainrand (Verlust).
**Tests**: `mend_the_bowl`, `the_bowl_calms_the_spirit`, `stake_out_a_new_boundary`,
`mend_view_shows_gap_and_pieces`.
**Status**: Complete

## Nacht und Morgen ([#4](../../../issues/4))

Ziel: Die C64-Fassung bekommt die zweite Konsequenzstufe der SDL-Fassung: Sumi fragt
nach der Übernachtung, der Futon ist bespielbar, jeder Ausgang zeigt am Morgen
Gewinn **und** Verlust, und am Schrein liegt die graue Spur samt Hinweis auf den
Besucher. Entschieden (mit Rainer, 2026-10-01):

- **Umfang:** volle Parität mit dem PC, alles nach dem Ausgang.
- **Ende:** der Ablauf folgt dem PC bis zur Teaser-Szene; danach kommt die vorhandene
  Schlusstafel („der Ausschnitt endet hier“). `OPEN_AFTER_OUTCOME` entfällt dann, die
  drei Sumi-Regeln führen auf beiden Seiten in die Nacht.
- **Geteilter Code:** Nacht, Frage und Dialogende entstehen von Anfang an in
  `shared/`, in der Form der C64-Fassung (direkter Zugriff auf `g->feld`, Bytetypen
  und Casts der C64); die PC-Fassung passt sich an. Die Texte und Tabellen kommen aus
  den `shared/*.inc`.
- **Platzgrenze:** höchstens 6 KB mehr in der `.prg` (Datei 28 246 Bytes vor Beginn,
  22 500 Bytes frei). Nach jeder Stufe `make clean build size`; wird die Grenze
  überschritten, wird angehalten und gefragt. Geschätzt: etwa 2,7 KB Text, mit Tabellen
  und Code 4 bis 5 KB (ungemessen).

## Stage 7: Phase und Morgenreaktionen der Figuren
**Ziel**: Die C64-Fassung kennt `Phase`; die Regelauswahl nutzt `g->phase` statt
`PHASE_ANY`. Sumi, Mio und Daigo antworten am Morgen je Ausgang anders (zehn Regeln, zehn
Dialoge, die Notiz `N_MORNING`); die Texte liegen einmal in `shared/*.inc`.
**Erfolgskriterien**: Mit `phase == PHASE_MORNING` und einem Ausgang antwortet jede der
drei Figuren mit ihrer Morgenzeile; mit `PHASE_BEFORE` bleibt es bei der bisherigen
Reaktion. Die Morgenregeln stehen nur noch in `shared/`. Größe gemessen.
**Tests**: `morning_reactions_follow_the_outcome`, `before_the_night_nothing_changes`.
**Status**: Complete

Ergebnis: C64-Datei **28 246 → 29 680 Bytes (+1 434)**, Code +57, Nur-Lese-Daten
+1 377 (zehn Dialoge, eine Notiz, zehn Regeln mit je 18 Bytes). Damit sind etwa 24 %
der 6-KB-Grenze verbraucht. Die Texte und die zehn Regeln stehen jetzt einmal in
`shared/` (`dialogue_rules_2_morning.inc`); der PC-eigene Block in `src/content.c`
ist weg. Die Includes heißen `_1`, `_2_morning`, `_3`, weil clang-format
aufeinanderfolgende `#include`-Zeilen alphabetisch ordnet und die Reihenfolge der
Regeln zählt.

## Stage 8: Die Übernachtungsfrage
**Ziel**: Nach Sumis Reaktion auf den Ausgang fragt das Spiel „Übernachten?“
(`GAME_PROMPT`, zwei Antworten); `spend_the_night` setzt den Morgen. Der Futon im
Gasthaus erklärt sich und fragt dasselbe.
**Erfolgskriterien**: Ja bringt in den Morgen im Gasthaus (Phase, Szene, Notiz); Nein oder
Abbrechen lässt es beim Spiel; solange der Wald unentschieden ist, geschieht nichts.
**Tests**: `the_night_is_offered_after_the_outcome`, `staying_awake_changes_nothing`,
`the_futon_asks_the_same`.
**Status**: Not Started

## Stage 9: Der Morgen in Dorf und Wald
**Ziel**: Morgen-Änderungen der Karte je Ausgang (`outcome_changes` mit Phase), der Fuchs ist
weg und der Bau leer, die graue Spur am Schrein, der späte Inschriftentext.
**Erfolgskriterien**: Jeder Ausgang zeigt am Morgen Gewinn und Verlust; die graue Spur ist
erst am Morgen zu sehen. Die Prüfung „keine Moral-Farben“ der PC-Tests gilt auch hier.
**Tests**: `the_morning_shows_gain_and_loss`, `the_grey_trace_comes_with_the_morning`.
**Status**: Not Started

## Stage 10: Teaser und Ende
**Ziel**: Das Lesen des Besuchers am Morgen öffnet die Teaser-Szene, danach die
Schlusstafel. Die Unterschiede `OPEN_AFTER_OUTCOME` und der Eintrag in der Erlaubnisliste
des Drift-Checks fallen weg.
**Erfolgskriterien**: Ablauf Ausgang → Nacht → Morgen → Teaser → Tafel in einem Test
durchgespielt; Driftmelder ohne Ausnahme für die drei Sumi-Regeln.
**Tests**: `the_whole_slice_to_the_end`.
**Status**: Not Started
