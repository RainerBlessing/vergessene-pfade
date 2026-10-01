# C64-POC „Die vergessenen Pfade“

Text-Modus-Fassung des Walddorf-Slice für den Commodore 64, gebaut mit
llvm-mos. Aufbau, Toolchain und VICE-Start sind von
`~/projekte/c64u/fps-monitor` übernommen.

**Umfang:** Erkundung von Dorf und Wald, die Begegnung am Hainrand und **alle
drei Ausgänge**: „bekämpfen“, „alte Grenze wiederherstellen“ und „Kintsugi und
Kompromiss“, dazu die Nacht im Gasthaus und der Morgen danach (ein Abend, eine
Nacht und ein Morgen, der zeigt, was die Entscheidung gekostet hat). Klang: kurze SID-Effekte, ohne eigenen Interrupt in der Hauptschleife (`src/sound.c`). Stand und
Stufen:
[IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md).

Die Regeln sind dieselbe Tabellenlogik wie in der SDL-Fassung (`../src`), nur
auf diesen Umfang gekürzt; die Texte sind wörtlich übernommen. Die Karten
stammen unverändert aus `../assets/maps`.

## Bauen und spielen

```sh
make            # build/vergessene-pfade.prg
make test       # Host-Tests (Regeln und Bildschirmaufbau)
make maps       # src/maps.h aus ../assets/maps neu erzeugen
make size       # was die .prg belegt, siehe „Speicher“
make bench      # was ein Bildaufbau kostet, siehe „Geschwindigkeit“
./run-vice.sh   # bauen und in VICE starten
```

`make` legt neben der `.prg` ein `build/spielen.sh` ab, das nur noch startet
und nichts baut — praktisch, um die fertige Fassung weiterzugeben oder schnell
hineinzuschauen:

```sh
./build/spielen.sh
```

Voraussetzung ist das llvm-mos-SDK unter `~/.local/share/llvm-mos-sdk`
(komplettes Release-Tarball, **nicht** das AUR-Paket `llvm-mos-bin`: dort fehlt
die `mos-platform`-Laufzeit). Die geprüfte Fassung ist **v23.2.0**, dieselbe,
die `.github/workflows/c64.yml` verwendet:

```sh
mkdir -p ~/.local/share/llvm-mos-sdk
curl -L -o /tmp/llvm-mos-linux.tar.xz \
  https://github.com/llvm-mos/llvm-mos-sdk/releases/download/v23.2.0/llvm-mos-linux.tar.xz
tar -xf /tmp/llvm-mos-linux.tar.xz -C ~/.local/share/llvm-mos-sdk --strip-components=1
```

Was lokal wirklich liegt, sagt `~/.local/share/llvm-mos-sdk/bin/clang --version`:
die Zeile nennt Fassung und llvm-mos-Commit. Ein selbst gebautes Rolling-SDK
baut zwar auch, ist aber nicht die Fassung, gegen die CI prüft — dann das
Verzeichnis leeren und die Befehle oben noch einmal laufen lassen.

In GitHub Actions läuft derselbe `make maps` / `make test` / `make build`; die
`.prg` hängt als Artefakt am Lauf. VICE ist dort bewusst nicht dabei: die
Emulatorläufe brauchen eine Anzeige und Wartezeiten und wären unzuverlässig.

## Steuerung

| Taste | Aktion |
|---|---|
| W A S D oder Cursortasten | gehen |
| RETURN / Leertaste | reden, untersuchen, weiterlesen |
| | untersucht wird, was in Blickrichtung liegt — sonst, was unter dir oder daneben liegt |
| I | Tasche; RETURN benutzt den gewählten Gegenstand |
| | eine Laufrichtung schließt die Tasche und geht den Schritt; W und S wählen nur, solange es etwas zu wählen gibt |
| N (oder RUN/STOP) | Notizbuch auf und zu |
| W/S und RETURN | in Begegnung, Tasche und Reparaturansicht wählen |

Steht ein Text unten im Bild, schließt **N** zuerst diesen Text; erst der
zweite Druck öffnet das Notizbuch. Im Notizbuch blättern W und S.

## Die alte Grenze

Der versetzte Grenzstein steht im Wald auf der Schleifspur. Schieben lässt er
sich erst, wenn **beides** gesehen wurde: die Schleifspur am Stein und die
leere Mulde weiter nördlich. Das Spiel sagt nicht, dass das zusammengehört.
Geschoben wird durch Gehen gegen den Stein, ein Feld je Schritt; rutscht er
irgendwo fest, rollt ihn ein Blick darauf an den Anfang der Spur zurück.
Liegt er wieder in der Mulde, zieht sich der Kami zurück, der Hain behält
seinen Rand, und das Lager verliert seinen besten Grund — Sumi und Daigo sagen
dazu Verschiedenes.

## Kintsugi und Kompromiss

Die lange Kette, jeder Schritt aus einer Beobachtung heraus: Scherben am
Opferstein aufheben → in der Tasche das eingebrannte Zeichen ansehen → dasselbe
Zeichen an Sumis Tür → Sumi erzählt von ihrer Großmutter → Oriha holt den
Goldlack. In der Reparaturansicht wird je ein Stück an die offene Bruchkante
gesetzt; was nicht passt, kostet nichts. Danach steht die Schale sichtbar in
Orihas Regal und trocknet — fertig ist sie nach der Rückkehr aus dem Wald, nicht
nach einem unsichtbaren Zähler. Die geflickte Schale dem Kami darbringen macht
ihn ruhig; erst dann, und nur mit Tierspuren und Auftragsbuch, geht Daigo mit
und steckt mit dir drei Pfähle entlang der Spuren ab (RETURN auf dem Feld, Daigo
muss daneben stehen). Dafür bekommt das Dorf Totholz, der alte Hainrand bleibt
verloren.

## Nacht, Morgen und Schluss

Nach dem Ausgang und Sumis Wort dazu fragt sie, ob man im Gasthaus übernachten will
(auch der Futon fragt, wenn man ihn ansieht). Wer „Uebernachten“ wählt und dessen
Wald entschieden ist, wacht am Morgen im Gasthaus auf: Jeder Ausgang zeigt noch
einmal Gewinn und Verlust, Sumi, Mio und Daigo antworten anders, der Fuchs ist
fort (oder hat Junge), und am Schrein liegt eine graue Spur. Wer sie liest und
danach die Inschrift, bekommt den Hinweis auf den Besucher und die Teaser-Szene.

Danach sagt der Ausschnitt, dass er zu Ende ist: eine Tafel mit dem Satz zum
gewählten Ausgang. Sie kommt einmal; danach läuft die Welt weiter, das Notizbuch
bleibt lesbar.

## Bild

40x25 Zeichen: Zeile 0 nennt den Ort, darunter liegt der scrollende
Kartenausschnitt (17 Zeilen), darunter die Texttafel. In der Begegnung wächst
die Tafel nach oben, damit die Handlungen daneben passen. Der Zeichensatz ist
der Kleinbuchstaben-Satz, damit die Texte lesbar bleiben.

## Speicher

`make size` liest das ELF neben der `.prg` und sagt, was wohin geht:

```
  Datei (mit Ladeadresse)   25968
  RAM belegt                26343  (40% von 64K)

  Gruppe                    Bytes
  Code                      11982
  Nur-Lese-Daten            13942
  Daten                        42
  BSS                         377
```

Die Nur-Lese-Daten sind zum größten Teil **Text**: rund 10 250 Bytes Dialoge,
Notizen und Tafeln, gegenüber rund 3 700 Bytes benannter Tabellen. Wer Platz
sucht, sucht ihn dort — nicht in den Tabellen. `make size` weist beides getrennt
aus und nennt die größten Symbole; `SIZE_TOP=40 make size` zeigt mehr davon.

Was gemessen und **übernommen** wurde (#25):

| Änderung | PRG |
|---|---|
| Ausgangspunkt | 29 801 |
| Karten lauflängenkodiert (2 688 Zellen → 1 114 Bytes + 128 Bytes Zeilentabelle) | −1 276 |
| Schadenszahlen und HP-Balken ohne 16-Bit-Division | −173 |
| `-Oz` statt `-Os` als Vorgabe (kostet 2,2 % Bildaufbau, siehe unten) | −2 384 |
| **Stand** | **25 968** |

Was gemessen und **nicht** übernommen wurde:

- **Dialoge als ein Textblock je Dialog** (Seiten durch `\f` getrennt, statt drei
  Seitenzeigern): spart 363 Bytes Tabelle, kostet 364 Bytes Code fürs Suchen der
  Seite. Auf dem 6502 ist Zeichenkette-durchlaufen teurer als der Zeiger, den es
  einspart — unterm Strich null.
- **`first_page + count` mit einer zentralen Seitentabelle** (der Vorschlag aus
  #25): spart 138 Bytes und bleibt beim direkten Zugriff. Dafür müssten die
  `first_page`-Werte für 73 Dialoge von Hand stimmen; jeder neue Dialog
  verschiebt alle folgenden. Für 0,5 % der `.prg` ist das die falsche Art von
  Handarbeit — richtig wird das erst mit einem Erzeuger, und der gehört zu
  [#11](../../../issues/11).
- **Umlaufende Auswahl ohne Modulo**: kostet 7 Bytes, weil `__umodhi3` für den
  Zufallswurf (`% 3`) sowieso im Programm steht.
- **LTO** ist bei llvm-mos die Vorgabe und lohnt deutlich: `OPT="-Oz -fno-lto"`
  ergibt 29 633 Bytes. Sie ist auch der Grund, warum in `make size` fast der
  ganze Code unter `main` steht — die Funktionen anderer Übersetzungseinheiten
  landen dort hineingezogen.

Keine dieser Änderungen setzt REU oder Ultimate-Hardware voraus.

## Geschwindigkeit

`make bench` baut `tools/bench_render.c` gegen dieselben Quellen wie das Spiel
(nur ohne Tastatur), zeichnet acht Bilder mit jeweils neuer Spielerposition und
zählt die Takte dazwischen mit den Timern von CIA#2. `tools/bench.py` startet
das in x64sc und holt das Ergebnis über den VICE-Monitor aus dem RAM. Der
Kernal-Interrupt ist während der Messung aus, die Badlines des VIC sind drin.
Die Zahl ist auf ±50 Takte reproduzierbar; `make bench` braucht eine Anzeige
(VICE ist mit GTK gebaut) und läuft darum nicht in CI.

Ein ganzes Bild neu zu zeichnen kostet **199 077 Takte**, also rund **200 ms**
bei 1 MHz. Das passiert einmal pro Tastendruck, nicht pro Bildwiederholung —
nichts hier hängt am Rasterstrahl. Wo die Takte liegen:

| Stand | Takte je Bild |
|---|---|
| vor #25 (unkomprimierte Karten, `-Os`) | 153 178 |
| mit lauflängenkodierten Karten, `-Os` | 194 824 |
| dasselbe mit `-Oz` (Vorgabe) | 199 077 |

- Die **Lauflängenkodierung** der Karten kostet **41 646 Takte (+27 %)** für
  1 276 Bytes: 17 Zeilen auspacken à rund 2 050 Takte (isoliert gemessen:
  34 866 Takte für 17 Zeilen). Der Preis ist höher als beim Einbau geschätzt.
  Er fällt, sobald ein Bild nicht mehr ganz neu gezeichnet wird — der Kartenteil
  ändert sich beim Gehen meist nur um zwei Zellen ([#28](../../../issues/28)).
- **`-Oz` statt `-Os`** kostet 4 253 Takte (+2,2 %) und spart 2 384 Bytes
  (8,4 % der `.prg`). Bei einmal pro Tastendruck ist das der bessere Tausch,
  darum ist `-Oz` die Vorgabe. Zum Nachrechnen: `make clean build size bench
  OPT=-Os`.

Drei Dinge tragen den Bildaufbau, die eine Neufassung nicht verlieren sollte:

- `tile_def()` schlägt das Symbol in einer 128-Byte-Tabelle nach, statt die
  Kachelliste zu durchsuchen (einmal pro gezeichneter Zelle).
- Die Karte wird direkt aus ihren Zeilen gezeichnet; Überschreibungen, Stein,
  Figuren und Spielfigur kommen danach obendrauf, statt pro Zelle abgefragt zu
  werden.
- Das Kartenfeld wird nur neu gezeichnet, wenn sich dort etwas ändern konnte
  (`map_is_current`). Weiterlesen in einem Dialog ist dadurch sofort da.

`map_row()` behält die letzte ausgepackte Zeile, damit die vielen
`map_at()`-Fragen auf derselben Zeile nichts kosten. Nachgezählt (mit einem
Zähler in `map_row()`, der nur zum Messen drin war): 19 Auspackvorgänge je Bild
für 17 Kartenzeilen — die Fragen nach Überschreibungen und Figuren kosten also
fast nichts extra.

## Prüfen

`make test` spielt die Regeln auf dem Host durch (`tests/test_game.c`) und
prüft den Bildschirmaufbau (`tests/test_screen.c`, mit `tests/c64.h` als
Ersatz für den SDK-Header). `make bench` sagt, was ein Bildaufbau auf der
emulierten Maschine kostet (oben). Für Läufe auf der echten Maschine hilft der
VICE-Monitor: `x64sc -remotemonitor -autostart build/vergessene-pfade.prg`,
dann über `127.0.0.1:6510` mit `m 0400 07e7` den Bildschirmspeicher auslesen
und mit `keybuf "..."` Tasten schicken. **Achtung:** Eine offene
Monitor-Verbindung hält die emulierte CPU an — für jede Messung neu verbinden
und die Verbindung dazwischen schließen.
