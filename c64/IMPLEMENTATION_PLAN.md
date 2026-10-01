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
**Ziel**: Der Futon im Gasthaus erklärt sich und fragt, ob man übernachtet (`GAME_PROMPT`,
zwei Antworten, eigener Bildschirm); `spend_the_night` setzt den Morgen. Die Logik steht
einmal in `shared/night.h`, die Zahlen und Texte in `shared/inn.h`.
**Erfolgskriterien**: Ja bringt in den Morgen im Gasthaus (Phase, Szene, Notiz,
Position); Nein oder Abbrechen lässt es beim Spiel; solange der Wald unentschieden
ist, schläft man nicht (`D_X_FUTON_AWAKE`). Ein Dialog mit `OPEN_NIGHT` fragt nach dem
Lesen bis zum Ende, nicht beim Abbrechen.
**Nicht Teil dieser Stufe**: Sumis drei Reaktionen führen weiter zur Schlusstafel
(`OPEN_AFTER_OUTCOME` ist auf der C64 noch `OPEN_END`). Die Frage nach Sumi kommt mit dem
Ende in Stufe 10; bis dahin erreicht man die Nacht über den Futon, und das Spiel bleibt in
jedem Zwischenstand stimmig.
**Tests**: `the_futon_asks_the_same`, `a_night_opened_by_a_dialogue_asks`,
`sleeping_brings_the_morning`, `staying_awake_changes_nothing`,
`an_unsettled_forest_gives_no_sleep`, `the_night_prompt_shows_question_and_answers`
(Bildschirm). Alle zuerst rot (Bau scheiterte an fehlenden Namen).
**Status**: Complete

Ergebnis: C64-Datei **29 680 → 30 646 Bytes (+966)**, insgesamt seit Beginn **+2 400**
(28 246 → 30 646), also 40 % der 6-KB-Grenze. Fünf Dialoge und zwei Untersuchungspunkte
stehen geteilt in `shared/` (`examine_points_futon.inc`). Der PC nutzt dieselben
Funktionen; sein Verhalten ist unverändert (seine Tests sind grün). **Beim Schließen
eines Dialogs** wichen die Fassungen ab (`OPEN_MEND`, `OPEN_FOLLOW`). Das wurde danach entschieden (siehe unten): es gilt das Verhalten des PC.

## Stage 9: Der Morgen in Dorf und Wald
**Ziel**: Morgen-Änderungen der Karte je Ausgang (jetzt mit `phase`), der Fuchs ist weg und der
Bau leer (mit Jungen nach dem Kompromiss), die graue Spur am Schrein, der späte
Inschriftentext.
**Erfolgskriterien**: Jeder Ausgang verwandelt am Morgen die Karte noch einmal; der Fuchs
bleibt am Tag des Kampfes und geht über Nacht; die graue Spur erscheint nach jedem Ausgang
und ihr Lesen führt zum Besucher am Schrein.
**Tests**: `the_fox_leaves_overnight_and_comes_back_with_kits`, `the_den_says_what_it_holds`,
`the_morning_changes_the_map_per_outcome`, `the_grey_patch_and_the_visitor_line`,
`the_morning_redraws_what_the_night_changed` (Bildschirm). Die ersten vier zuerst rot.
**Status**: Complete

Ergebnis: C64-Datei **30 662 → 31 484 Bytes (+822)**, insgesamt seit Beginn **+3 238**
(28 246 → 31 484), also 54 % der 6-KB-Grenze. Was neu geteilt ist: fünf Dialoge, drei Notizen,
fünf Untersuchungspunkte (`examine_points_2_late.inc`, `_4_morning.inc`) und **alle**
Kartenänderungen und Spuren: `tile_changes.inc` und `tile_overrides.inc` benutzen Makros
(`CHANGE`, `MARK`, `TRACK`, `TILE`); jede Fassung sagt, was `TILE` ist (PC mit `tag`, C64
mit `phase`). Auf dem PC bleiben zwei Tabellen (`outcome_changes` zuerst), auf der C64 steht
eine, in derselben Reihenfolge. Drei neue Kacheln auf der C64 (`g`, `j`, `v`; die Zeichen
sind eine Wahl, nicht aus dem PC übernommen).

## Stage 10: Teaser und Ende
**Ziel**: Sumi fragt nach ihrem Wort zum Ausgang nach der Nacht; das Lesen des Besuchers am
Morgen öffnet die Teaser-Szene, danach kommt die Schlusstafel. Die Unterschiede
`OPEN_AFTER_OUTCOME` und der Eintrag in der Erlaubnisliste des Drift-Checks fallen weg.
**Erfolgskriterien**: Ausgang → Sumi → Frage → Nacht → Morgen → graue Spur → Besucher →
Teaser → Tafel in einem Test durchgespielt; die Tafel kommt einmal; ohne den Morgen ist der
Besucher nur eine Zeile. Driftmelder ohne Ausnahme in den Regeln.
**Tests**: `the_slice_says_when_it_ends` (umgeschrieben auf den ganzen Ablauf),
`the_visitor_ends_it_only_in_the_morning`, `the_closing_panel_names_the_ending`. Zuerst rot.
**Status**: Complete

Ergebnis: C64-Datei **31 484 → 31 920 Bytes (+436)**, insgesamt seit Beginn **+3 674**
(28 246 → 31 920), also 60 % der 6-KB-Grenze (Rest 2 470 Bytes). Die Dialog-Schlussbehandlung
steht ganz geteilt in `opens_after_dialogue()`; `OPEN_END` gibt es nicht mehr, der Teaser
(`OPEN_TEASER`) ist auf beiden Seiten gleich. `OPEN_AFTER_OUTCOME` und die drei Einträge der
Erlaubnisliste sind weg; die Regeln beider Fassungen sind jetzt gleich. Die Schlusstafel
sagte noch „in der vollen Fassung folgt eine Nacht und ein Morgen“ und musste neu
formuliert werden (Wortlaut von mir, bitte prüfen). Beim Schreiben der Tests fiel auf: Das
Lesen des Besuchers merkte sich `OBS_TEASED` schon, wenn die Zeile aufging, nicht erst nach
dem Lesen. Wer sie mit Escape schloss, hatte Teaser und Tafel für immer verpasst; auf dem PC
war das schon so. **Behoben (auf Hinweis aus dem Review):** `OBS_TEASED` wird erst gesetzt,
wenn der Teaser wirklich öffnet (`opens_after_dialogue()`); wer die Zeile mit Escape
schließt, bekommt Teaser und Tafel beim nächsten Lesen. Tests auf beiden Seiten
(`the_visitor_ends_it_only_in_the_morning`, `test_escape_keeps_the_teaser`; beide zuerst rot).
Das ändert auch das Verhalten des PC. C64-Datei danach 31 749 Bytes (−171 gegenüber 31 920; der
Grund ist nicht untersucht, vermutlich andere Einbettung unter LTO), insgesamt +3 503.

## Entschieden (aus Stufe 8): Escape beim Schließen eines Dialogs
`game_action` schloss einen Dialog auf PC und C64 verschieden: Auf dem PC öffnete Escape
trotzdem die Reparaturansicht (`OPEN_MEND`) und ließ Daigo folgen (`OPEN_FOLLOW`), auf der
C64 geschah das nur nach dem Lesen bis zum Ende. **Entschieden (Rainer, 2026-10-01):** Es
geschieht auch ohne das Lesen, wie auf dem PC. Gelesen werden muss nur für die Nacht und
das Ende (bzw. den Teaser). Die Behandlung steht einmal in `opens_after_dialogue()`
(`shared/night.h`); die C64 hat sich angepasst. Tests auf beiden Seiten
(`escape_still_opens_what_the_dialogue_opens`, `test_escape_still_opens`; der C64-Test war
zuerst rot).
