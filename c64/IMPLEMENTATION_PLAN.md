# C64-POC „Die vergessenen Pfade“

Ziel: prüfen, ob der Loop **Erkunden → Beobachten → Verstehen → Handeln** in
40x25 PETSCII-Zeichen und auf einer 1-MHz-Maschine trägt. Vorlage für Aufbau,
Toolchain und VICE-Start ist `~/projekte/c64u/fps-monitor`.

Umfang (mit Rainer abgestimmt): Erkundung beider Karten, die Begegnung am
Hainrand und die Ausgänge „bekämpfen“ und „alte Grenze“. Nicht im POC:
Kintsugi, Nacht im Gasthaus, Morgen-Phase, Klang.

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
