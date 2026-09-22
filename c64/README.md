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
./run-vice.sh   # bauen und in VICE starten
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

In GitHub Actions läuft derselbe `make maps` / `make test` / `make build`; die
`.prg` hängt als Artefakt am Lauf. VICE ist dort bewusst nicht dabei: die
Emulatorläufe brauchen eine Anzeige und Wartezeiten und wären unzuverlässig.

## Steuerung

| Taste | Aktion |
|---|---|
| W A S D oder Cursortasten | gehen |
| RETURN / Leertaste | reden, untersuchen, weiterlesen |
| I | Tasche; RETURN benutzt den gewählten Gegenstand |
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

## Bild

40x25 Zeichen: Zeile 0 nennt den Ort, darunter liegt der scrollende
Kartenausschnitt (17 Zeilen), darunter die Texttafel. In der Begegnung wächst
die Tafel nach oben, damit die Handlungen daneben passen. Der Zeichensatz ist
der Kleinbuchstaben-Satz, damit die Texte lesbar bleiben.

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

## Prüfen

`make test` spielt die Regeln auf dem Host durch (`tests/test_game.c`) und
prüft den Bildschirmaufbau (`tests/test_screen.c`, mit `tests/c64.h` als
Ersatz für den SDK-Header). Für Läufe auf der echten Maschine hilft der
VICE-Monitor: `x64sc -remotemonitor -autostart build/vergessene-pfade.prg`,
dann über `127.0.0.1:6510` mit `m 0400 07e7` den Bildschirmspeicher auslesen
und mit `keybuf "..."` Tasten schicken. **Achtung:** Eine offene
Monitor-Verbindung hält die emulierte CPU an — für jede Messung neu verbinden
und die Verbindung dazwischen schließen.
