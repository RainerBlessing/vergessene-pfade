# C64-POC „Die vergessenen Pfade“

Text-Modus-Fassung des Walddorf-Slice für den Commodore 64, gebaut mit
llvm-mos. Aufbau, Toolchain und VICE-Start sind von
`~/projekte/c64u/fps-monitor` übernommen.

**Umfang:** Erkundung von Dorf und Wald, die Begegnung am Hainrand und **alle
drei Ausgänge**: „bekämpfen“, „alte Grenze wiederherstellen“ und „Kintsugi und
Kompromiss“. Nicht enthalten: Nacht im Gasthaus, Morgen-Phase, Klang. Stand und
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

## Schluss

Nach dem Ausgang und Sumis Wort dazu sagt der Ausschnitt, dass er zu Ende ist:
eine Tafel mit dem Satz zum gewählten Ausgang und dem, was in der vollen Fassung
folgen würde (Nacht, Morgen, die Spur am Schrein). Sie kommt einmal; danach läuft
die Welt weiter, das Notizbuch bleibt lesbar.

## Bild

40x25 Zeichen: Zeile 0 nennt den Ort, darunter liegt der scrollende
Kartenausschnitt (17 Zeilen), darunter die Texttafel. In der Begegnung wächst
die Tafel nach oben, damit die Handlungen daneben passen. Der Zeichensatz ist
der Kleinbuchstaben-Satz, damit die Texte lesbar bleiben.

## Speicher

`make size` liest das ELF neben der `.prg` und sagt, was wohin geht:

```
  Datei (mit Ladeadresse)   28352
  RAM belegt                28737  (43% von 64K)

  Gruppe                    Bytes
  Code                      14366
  Nur-Lese-Daten            13948
  Daten                        36
  BSS                         387
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
| **Stand** | **28 352** |

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
- **`-Oz`**: `make clean build size OPT=-Oz` ergibt 25 968 Bytes, also 2 384
  weniger. Nicht als Vorgabe übernommen, weil `-Oz` gegen Inlining arbeitet und
  der Bildaufbau (unten) die einzige Stelle ist, an der das weh tut — und diese
  Messung braucht VICE. Bis dahin ist `-Oz` nur ein Schalter.
- **LTO** ist bei llvm-mos die Vorgabe und lohnt deutlich: `OPT="-Os -fno-lto"`
  ergibt 30 578 Bytes. Sie ist auch der Grund, warum in `make size` fast der
  ganze Code unter `main` steht — die Funktionen anderer Übersetzungseinheiten
  landen dort hineingezogen.

Keine dieser Änderungen setzt REU oder Ultimate-Hardware voraus.

## Geschwindigkeit

Ein ganzes Bild neu zu zeichnen kostet rund **36 000 Takte** (~36 ms bei 1 MHz,
gemessen mit dem VICE-Monitor über `watch store $07e7` und `stopwatch`). Dafür
nötig waren drei Dinge, die eine Neufassung nicht verlieren sollte:

- `tile_def()` schlägt das Symbol in einer 128-Byte-Tabelle nach, statt die
  Kachelliste zu durchsuchen (einmal pro gezeichneter Zelle).
- Die Karte wird direkt aus ihren Zeilen gezeichnet; Überschreibungen, Stein,
  Figuren und Spielfigur kommen danach obendrauf, statt pro Zelle abgefragt zu
  werden.
- Das Kartenfeld wird nur neu gezeichnet, wenn sich dort etwas ändern konnte
  (`map_is_current`). Weiterlesen in einem Dialog ist dadurch sofort da.

Seit #25 liegen die Karten komprimiert im Programm, also kommt pro gezeichneter
Zeile ein Auspacken dazu: 17 Zeilen à höchstens 48 Zellen, geschätzt einige
Tausend Takte auf die 36 000. `map_row()` behält die letzte Zeile, damit die
vielen `map_at()`-Fragen auf derselben Zeile nichts kosten. Nachgemessen ist das
noch nicht — dafür braucht es wieder den VICE-Monitor.

## Prüfen

`make test` spielt die Regeln auf dem Host durch (`tests/test_game.c`) und
prüft den Bildschirmaufbau (`tests/test_screen.c`, mit `tests/c64.h` als
Ersatz für den SDK-Header). Für Läufe auf der echten Maschine hilft der
VICE-Monitor: `x64sc -remotemonitor -autostart build/vergessene-pfade.prg`,
dann über `127.0.0.1:6510` mit `m 0400 07e7` den Bildschirmspeicher auslesen
und mit `keybuf "..."` Tasten schicken. **Achtung:** Eine offene
Monitor-Verbindung hält die emulierte CPU an — für jede Messung neu verbinden
und die Verbindung dazwischen schließen.
