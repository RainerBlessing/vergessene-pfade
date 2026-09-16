# Die vergessenen Pfade – POC „Das Walddorf“

Narrativer Retro-RPG-Prototyp (C11 / SDL3). Er prüft, ob der Loop
**Erkunden → Beobachten → Verstehen → Handeln → Welt verändert sich** in rund
30 Minuten trägt.

- Plan und Stand: [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)
- Spielbares Design: [docs/walddorf-slice.md](docs/walddorf-slice.md)
- Kulturelle Notizen und Quellen: [docs/culture-notes.md](docs/culture-notes.md)
- Herkunft des Codes: Emberpost (Japan-/Deutsch-Fassung), siehe [docs/emberpost/](docs/emberpost/)

**Aktueller Stand (Stage 2B):**
- Das Spiel beginnt mit der Ankunftsszene im Wald. Danach sind das Dorf Kiriyama und der Wald begehbar.
- Fünf Figuren sprechen nach einer Regeltabelle, die Umgebung lässt sich untersuchen, und Beobachtungen landen diegetisch im Notizbuch.
- Der verletzte Fuchs lässt sich mit Mios Heilkraut versorgen. Danach sieht die Figur Tierfährten („Tierzeichen“).
- Wer den Hain betritt, wird von einem Windstoß zurückgeworfen.
- Die Begegnung mit dem Kami und die Lösungswege folgen ab Stage 3A.

## Bauen (Linux)

Voraussetzungen: CMake ≥ 3.20, C11-Compiler, SDL3-Entwicklungspaket (z. B. Arch: `sdl3`).

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/vergessene_pfade
```

Ohne System-SDL3 kann das SDL-3.2.28-Quellarchiv nach `vendor/SDL3-3.2.28`
entpackt und mit `-DVP_VENDORED_SDL=ON` gebaut werden. Das ist später der Weg für
das statisch gelinkte Windows-Paket (Stage 6). `vendor/` wird nicht eingecheckt.

Nur den SDL-freien Core und die Tests bauen:

```sh
cmake -S . -B build-core -DVP_BUILD_GAME=OFF
cmake --build build-core && ctest --test-dir build-core
```

## Diagnose

Alle Modi aus einem beschreibbaren Arbeitsverzeichnis starten:

- `vergessene_pfade --smoke`: 10 Frames rendern, `smoke.bmp` schreiben, beenden.
- `vergessene_pfade --verify`: den kompletten Journey-Test im echten Fenster
  spielen und nummerierte BMP-Screenshots schreiben. Exit 0 nur bei Erfolg.
- `vergessene_pfade --log sitzung.log`: Sitzungsprotokoll für Playtests, auch
  zusammen mit `--verify`. Jede Zeile hat die Spalten
  `zeit_ms  karte  x,y  ereignis  details`.
  - Ereignisse: `observe`, `examine`, `examine_nothing`, `npc_talk`, `notebook_open`,
    `knockback`, `action_attempt` (Gegenstand benutzt, `ok=0/1`).
  - Wiederholte erfolglose Untersuchungen desselben Ziels erzeugen keine eigene
    Zeile. Ihre Anzahl steht als `repeat=n` in der nächsten
    `examine_nothing`-Zeile.

## Steuerung

| Taste | Aktion |
|---|---|
| Pfeile / WASD | gehen |
| Leertaste / Enter | reden bzw. untersuchen (Blickfeld, sonst das Feld unter dir), weiter |
| I | Tasche; Enter benutzt einen Gegenstand am Ziel vor dir oder sieht ihn genauer an |
| Esc | Dialog schließen / Notizbuch (Hoch/Runter blättert) |
| F1 / F2 | Debug-Anzeige / Hindernisse |
| R, Q (im Notizbuch) | neu starten / beenden |

## Formatierung

`clang-format -i src/*.[ch] tests/*.[ch]` (Konfiguration in `.clang-format`).
