# IMPLEMENTATION_PLAN – POC „Die vergessenen Pfade“: Vertical Slice „Das Walddorf“

Quelle: `Die_vergessenen_Pfade_Game_Concept.pdf` (Review-Fassung 09/2026), Kapitel 7.
Spielbares Design des Slices: [docs/walddorf-slice.md](docs/walddorf-slice.md).

## Ziel des POC

Ein etwa 30 Minuten langer, spielbarer PC-Prototyp beantwortet eine Frage: **Trägt
der Loop Erkunden → Beobachten → Verstehen → Handeln → Welt verändert sich, bevor
daraus ein großes RPG wird?** Maßstab sind die Abnahmekriterien aus dem Konzept
(siehe „Playtest & Messung“).

Explizit **nicht** Teil des POC: weitere Regionen, Crafting-System, Skilltree,
Speichern/Laden, generisches RPG-Framework, eigene Font-Entwicklung. Windows-Build
und C64U-Evaluierung kommen erst nach positivem Playtest (Stage 6).

## Designregeln (für jede Stufe verbindlich)

1. **Beobachtung ≠ Schlussfolgerung.** Das Spiel speichert nur *Beobachtungen*
   (`obs`-Bits). Es setzt nie eine Erkenntnis wie „Der Kami schützt den Hain“.
   Beobachtungen schalten stattdessen **Handlungen** frei. Ob die Person verstanden
   hat, zeigt ihr Verhalten und das Playtest-Interview.
2. **Das Notizbuch ist diegetisch.** Es enthält kurze Beschreibungen in der Stimme
   der Spielfigur, z. B. „Der Fuchs ist verletzt. Seine Pfote trägt dunklen Lehm.“
   Es gibt keine Häkchen, keine Zähler („3/5“), keine offenen Punkte und keine
   Bewertungen. Das einzige Ziel ist das anfängliche „Sumi bat mich, den Geist
   zu vertreiben.“
3. **Kein Ausgang ist das gute Ende.** Jeder Ausgang hat sichtbaren Gewinn *und*
   sichtbaren Verlust, jeweils sofort und am nächsten Morgen. Die Präsentation ist
   gleich: keine Moral-Farben, kein Bonus. Ein Test prüft das an den Daten.
4. **Riten folgen aus dem Ort, nicht aus „Magie“.** Es gibt keine echten
   Ritualgegenstände als Zauberwerkzeug (z. B. keine Ofuda als Siegel). Handlungen
   leiten ihre Bedeutung aus der Geschichte des Ortes ab, etwa der alten Grenze
   oder der Schale einer Schreinhüterin.
5. **Der Leere Gast wird nicht erklärt.** Es gibt nur eine unerklärliche Spur und
   einen Besucher in einer Inschrift, keinen Namen und keine Deutung.

## Ausgangsbasis: vorhandene Elemente

Die Basis ist **Emberpost – Japan-/Deutsch-Fassung**
(`/mnt/C/Users/Rainer/Documents/Codex/2026-09-15/files-pasted-by-the-user-task/outputs/emberpost`).
`work/clean-source` (älterer, englischer Stand) und die Patch-Skripte in `work/`
werden nicht verwendet.

| Element in Emberpost | Verwendung im POC |
|---|---|
| C11 + SDL3, CMake, SDL-freier `game_core` | **übernehmen** (hält die C64U-Option offen) |
| `main.c` (Plattformschicht, Eingabe, `--verify`/`--smoke`) | **übernehmen**, um `--log` erweitern |
| `world.c` (Kartenparser, Tile-Tabelle, Kollision), `game_camera` | **übernehmen**, um neue Tiles, schiebbare Objekte und Tile-Overrides erweitern |
| `renderer.c` (320×200, Integer-Skalierung, Papier-Dialogfenster, Journal) | **übernehmen**, um Tiere, Kami, Fährten, Reparaturansicht und Notizbuch erweitern |
| `assets/tiles/generate_atlas.py` (eigene 16×16-Palette) | **übernehmen und erweitern** |
| Dialogsystem (Seiten, feste Umbrüche, Breitentest) | **übernehmen**, um Regeltabelle erweitern |
| `combat.c` (rundenbasiert, geseedetes LCG) | **umbauen** zum Begegnungssystem mit Gemütszuständen |
| `quest.c` | **ersetzen** durch Beobachtungs-Bitmaske, Phase und Ausgang |
| `inventory.c` | **verkleinern** auf Schlüsselgegenstände |
| Laden, Gold, Reiseklinge | **entfernen** |
| `tests/test_game.c` (CHECK-Makro), `tests/journey.c` (Pfadfinder nur über `game_action`) | **übernehmen**; ein Journey-Test pro Ausgang |
| `docs/validation.txt`, Milestone-Log | **übernehmen** als Vorlage |

**nippon-museum.de** (Fan-Archiv zum C64-RPG *NIPPON*, 1986/87) dient nur als
Strukturreferenz für Retro-RPGs und später für die C64U-Frage. Assets, Texte,
Karten und Namen werden nicht übernommen. Für kulturelle Inhalte ist die Seite
keine Quelle.

## Inhaltliche Inspiration

**Hiroko Yoda, Sofia Blind: „Acht Millionen Wege zum Glück – Eine faszinierende
Reise zu Japans spirituellem Erbe“ (dtv, ISBN 978-3-423-26470-9).** Der Titel
spielt auf *yaoyorozu no kami* an, die „acht Millionen Kami“: Orte, Bäume, Steine
und Dinge können beseelt sein. Das ist der thematische Kern des Walddorfs.

- Das Buch ist **Hintergrundrecherche, keine Vorlage**. Figuren, Texte, Anekdoten
  und Handlung werden nicht übernommen.
- Rainer hält ab Stage 1 Notizen mit Seitenangabe in `docs/culture-notes.md` fest.
  Die Tabelle „Motiv → Mechanik“ steht in der Slice-Spec.
- Für die Sensitivity-Prüfung reicht das Buch allein nicht, weil es sich an ein
  westliches Publikum richtet. Vor den externen Playtests (5B) kommen Fachquellen dazu.

## Technische Leitplanken

- Zuerst entwickelt wird unter Linux. SDL3 3.4.16 ist systemweit vorhanden.
  CMake nutzt `find_package(SDL3)` mit vendortem SDL 3.2.28 als Fallback
  (`VP_VENDORED_SDL`).
- Der Core bleibt SDL-frei: keine Allokationen pro Frame, feste Tabellen,
  Beobachtungen als `uint32_t`-Bitmaske, Texte über IDs.
- Inhalte liegen in C-Tabellen (`content_walddorf.c`), Karten in externen Textdateien.
  Es gibt keine Skriptsprache und kein ECS.
- Bedingungen sind **Daten**:
  - Dialogregeln `(npc, benötigt, verboten, ausgang, phase) → dialog`
  - Untersuchungspunkte `(karte, x, y, benötigt) → beobachtung + text`
  - Handlungsfreischaltungen `(ziel, benötigt, gegenstand) → aktion`
  - Tile-Overrides `(ausgang, phase) → (karte, x, y, symbol)`
- Texte werden als ASCII-Umschrift geschrieben („ae“, „ue“, „ss“), es gibt keine
  Font-Arbeit im POC.
- **Agenten-Regel:** Jede Stufe liefert *einen* spielbaren Zuwachs. Sie endet mit
  einem Build ohne Warnungen (`-Wall -Wextra -Wpedantic`), grünem `ctest`, grünem
  Journey-Test und angesehenen `--verify`-Screenshots. Die nächste Stufe beginnt
  erst danach. Es werden nie mehrere halbfertige Systeme parallel gebaut.

## Session-Log (ab Stage 2A)

`--log <datei>` schreibt eine Zeile pro Ereignis:
`zeit_ms  karte  x,y  ereignis  details`. Ereignisse:

| Ereignis | Details |
|---|---|
| `observe` | Beobachtungs-ID (nur beim ersten Fund) |
| `examine_nothing` | Tile-Symbol; **erfolglose Untersuchung** zeigt, wo die Welt etwas verspricht, das sie nicht hält. Geloggt wird nur, wenn sich Karte, Zielfeld oder Blickrichtung seit der letzten erfolglosen Untersuchung geändert haben, damit wiederholtes Drücken das Log nicht dominiert. Die unterdrückten Wiederholungen stehen als `repeat=n` in der nächsten Zeile. |
| `npc_talk` | NPC, gewählte Dialog-ID |
| `action_unlocked` | Aktions-ID |
| `action_attempt` | Aktion, Erfolg/Fehlschlag, Grund |
| `encounter_action` | Aktion, Gemüt vorher → nachher |
| `outcome` | Ausgang |
| `phase` | neue Phase |
| `notebook_open` | Anzahl Einträge |

Der Log wird im Core als Ereignispuffer geführt; `main.c` schreibt die Datei.

---

## Stage 1: Projektbasis & Slice-Spezifikation
**Goal**: Neues Projekt `~/projekte/own/cpp/vergessene-pfade` aus Emberpost-Japan, unter Linux baubar und entschlackt; Design-Spec abgenommen.
**Success Criteria**:
- `git init`, Quellen aus `outputs/emberpost` kopiert (ohne `build/`), umbenannt (Target `vergessene_pfade`, Titel „DIE VERGESSENEN PFADE“).
- CMake mit System-SDL3 und vendortem Fallback; `VP_BUILD_GAME=OFF` baut nur den Core.
- Laden, Gold, Reiseklinge und `GAME_SHOP` entfernt, Tests angepasst.
- `docs/walddorf-slice.md` von Rainer abgenommen.
- `docs/culture-notes.md` angelegt (Quellen, offene Fragen).
**Tests**:
- `ctest`: `core` und `complete_journey` grün.
- `./vergessene_pfade --smoke` unter Linux mit Exit 0.
**Status**: Complete (2026-09-16). Linux/GCC mit System-SDL3 3.4.16: Build ohne Warnungen, `ctest` 2/2, `--smoke` und `--verify` mit Exit 0, 10 Screenshots angesehen. Die Notizbuch-Überlappung aus Emberpost ist behoben. Vendortes SDL ist noch nicht eingecheckt (Stage 6).

## Stage 2A: Erkunden – Dorf, Wald, Untersuchen, Notizbuch
**Goal**: Dorf und Wald sind begehbar, Umgebung und NPCs lassen sich untersuchen, Beobachtungen landen diegetisch im Notizbuch.
**Success Criteria**:
- `village.map` (≈32×24) und `forest.map` (≈48×40) laut Spec. Fuchs und Fährten fehlen noch.
- 5 NPCs, Dialoge über die Regeltabelle.
- **Untersuchen**: Enter auf ein Tile ohne NPC prüft die Untersuchungstabelle und setzt die Beobachtung. Ohne Treffer kommt ein neutraler Text und ein Eintrag `examine_nothing` im Log.
- Notizbuch nach Designregel 2.
- `--log` gemäß Session-Log.
**Tests**:
- Unit: Beobachtung setzen ist idempotent; `examine_nothing` ohne Zustandsänderung und ohne doppelte Logzeile bei gleichem Zielfeld; Dialogregeln (erste Übereinstimmung gewinnt, `verboten` greift).
- Unit: Alle Texte passen ins Panel.
- Unit: **Notizbuch-Guard**: Kein Notizbuchtext enthält Ziffern, „/“ oder Häkchen-Zeichen.
- Journey: Alle Untersuchungspunkte von 2A sind vom Start aus erreichbar.
**Status**: Complete (2026-09-16).
- Build ohne Warnungen, `ctest` 2/2.
- `--verify` und `--log` mit Exit 0; 10 Screenshots angesehen (Dorf, Dialog, Hausmarke, Wald, Scherben, Tasche, Mulde, Besitzerin, Notizbuch, Debug).
- Probeweise eingebaute Fehler (Regel `verboten` ignoriert, Log-Deduplizierung aus) werden von den Tests erkannt.
- Anmerkungen:
  - Passierbare Untersuchungspunkte (Mulde, Schleifspur) werden unter der Figur untersucht, da man sie nicht „anblicken“ kann, ohne sie zu betreten.
  - Das Kampfmodul (`combat.c`) bleibt mit Unit-Test erhalten, ist aber bis Stage 3A nicht ans Spiel angeschlossen.

## Stage 2B: Wahrnehmung – Fuchs, Tierzeichen, Gefahr
**Goal**: Das Versorgen des Fuchses verändert, was die Spielfigur *sehen* kann. Der Hainrand ist gefährlich.
**Success Criteria**:
- Verletzter Fuchs (Sprite) am Bau; Heilkraut von Mio benutzen → Fuchs versorgt → Fähigkeit **Tierzeichen**.
- Fährten-Tiles werden nur mit Tierzeichen gerendert und sind nur dann untersuchbar.
- Gefahrenzone am Hainrand: Der Kami stößt die Spielfigur zurück. Das ist Spannung ohne Kampfzwang.
- Ankunftsszene: Kenta flieht, der Spieler wird einmal zurückgestoßen.
**Tests**:
- Unit: Fährten unsichtbar/ununtersuchbar ohne Fähigkeit, sichtbar mit Fähigkeit; Fuchs nur einmal versorgbar; Rückstoß landet auf einem begehbaren Tile.
- Journey: Start → Mio → Fuchs → Fährte gefunden.
**Status**: Complete (2026-09-16).
- Build ohne Warnungen, `ctest` 2/2.
- `--verify --log` und `--smoke` mit Exit 0; Screenshots angesehen (Szene, Fuchs, Versorgung, Fährten, Rückstoß, Notizbuch).
- Probeweise eingebaute Fehler (Rückstoß aus, Überlagerungen ohne Bedingung) werden erkannt.
- Umsetzung:
  - Gegenstände werden über die Tasche am Ziel in Blickrichtung benutzt (`POINT_ITEM` mit Symbol, verbraucht `takes`).
  - Versorgter Fuchs und Fährten sind **Tile-Overrides** mit Beobachtungsbedingung; Stage 4 erweitert sie um Ausgang und Phase.
  - Moosboden ist `guarded`: Der Hain ist vor einer Auflösung nicht betretbar. Deshalb steht der alte Baum jetzt am Hainrand (22,11).
  - Das Spiel beginnt im Wald mit der Ankunftsszene.
  - Neue Log-Ereignisse: `knockback`, `action_attempt`.

## Stage 3A: Begegnungssystem
**Goal**: Der Kami ist ein Wesen mit Gemüt, nicht nur ein Lebenspunktebalken.
**Success Criteria**:
- Zustand `GAME_ENCOUNTER`, ausgelöst am Hainrand per Tabelle.
- Gemüt `WUETEND / MISSTRAUISCH / RUHIG`, Übergangstabelle je Aktion (`encounter.c`, Umbau von `combat.c`).
- Aktionen: Zurückweichen, Warten, Darbringen (bei passendem Gegenstand), Angreifen. Verfügbarkeit kommt aus der Freischaltungstabelle.
- Ungeeignete Aktionen haben Folgen (Gemüt kippt), statt ausgegraut zu sein.
- Noch ohne Ausgang: Die Begegnung endet mit Zurückweichen.
**Tests**:
- Unit: vollständige Übergangstabelle; deterministisch; Encounter-Ende setzt keinen Ausgang.
- Journey: Begegnung betreten und verlassen.
**Status**: Complete (2026-09-18).
- Build ohne Warnungen, `ctest` 2/2; `--verify --log` mit Exit 0, Screenshots angesehen.
- Probeweise eingebaute Fehler (Gemüt ändert sich nicht, Darbringen ohne Gegenstand wählbar) werden erkannt.
- **Nachtrag aus dem Review von 2B/3A (2026-09-18):** Die Meldung „Build ohne Warnungen“ für 2B und 3A war falsch. Das neue `guarded`-Feld erzeugte 35 `-Wmissing-field-initializers` in `world.c`, die meine gefilterte Ausgabe verdeckt hatte. Behoben durch benannte Initialisierer; der Build ist jetzt nachweislich warnungsfrei. Weitere Korrekturen: Escape sucht „Zurückweichen“ in der Liste (statt den letzten Eintrag zu nehmen), Puffergröße über `ENCOUNTER_OPTION_LIMIT` mit `_Static_assert`, `show()` ohne NULL-Text, Mio gibt Ersatz-Heilkraut (sonst Softlock, sobald der Kampf Schaden macht), drei Testfehler.
- Abweichung vom Plan: **Angreifen wird noch nicht angeboten.** Ohne Kampf wäre es nur ein Platzhaltertext. Die Aktion steht bereits in der Gemüts- und Texttabelle; Stage 3B ergänzt eine Zeile in `encounter_options`.
- Umsetzung:
  - Der Hainrand löst jetzt die Begegnung aus. Das Zurückweichen (nicht mehr ein blinder Windstoß) schiebt die Figur auf sicheren Boden; das Ereignis `knockback` entfällt.
  - Das Gemüt bleibt zwischen Begegnungen erhalten.
  - Darbringen zeigt den Gegenstand im Menü; die Scherben sind ein Fehlgriff mit Folge (Gemüt wird zornig, Notizbucheintrag).
  - Neue Log-Ereignisse: `encounter`, `encounter_action`.

## Stage 3B: Lösung „Bekämpfen“
**Goal**: Erster vollständiger Ausgang.
**Success Criteria**:
- Rundenbasierter Kampf im Begegnungssystem (Schaden über das geseedete LCG aus Emberpost). Das Gemüt beeinflusst den Schaden.
- Sieg → Ausgang `OUT_FIGHT`. Niederlage → Rückkehr ins Dorf, kein Ausgang.
- Minimale Konsequenz: ein Dialogwechsel bei Sumi (volle Konsequenzen folgen in Stage 4).
**Tests**:
- Unit: Sieg/Niederlage, Ausgang wird nur einmal gesetzt.
- `journey_fight` grün; alle bisherigen Journeys grün.
**Status**: Complete (2026-09-18).
- Build ohne Warnungen (`grep -c warning` = 0), `ctest` 2/2, `--verify --log` und `--smoke` mit Exit 0, Screenshots angesehen.
- Probeweise eingebaute Fehler (Wunden des Kami vergessen, Hain bleibt nach dem Sieg bewacht) werden erkannt.
- **Nachtrag aus dem Review von 3B (2026-09-18):** Sieben Befunde behoben. Wichtig: Die vierte Option überschrieb die Fußzeile des Begegnungsfensters; die Reaktion auf einen Ausgang verdeckte Sumis Auftrag und die Schalen-Geschichte dauerhaft (jetzt stehen beide Regeln davor, mit Test); nach einer Niederlage blieb die Blickrichtung diagonal; die Niederlage nutzt jetzt die Übergangstabelle statt fester Koordinaten und nennt das richtige Tor; die Zahlen der letzten Kampfrunde waren unsichtbar (eine leere Textseite zeigt jetzt die letzte Meldung); ein ungenutzter Dialog wurde entfernt; `--verify --log` trennt die Durchläufe und sichert Ereignisse vor dem Neustart.
- Umsetzung:
  - „Angreifen“ ist jetzt in `encounter_options`; „Heilkraut nehmen“ erscheint nur im Kampf und nur mit Kraut. Die Optionen unterscheiden über `OptionWhen` zwischen Begegnung und Kampf.
  - Der Kami trifft im Zustand ZORNIG härter (`rage`).
  - Zurückweichen pausiert den Kampf: Die Wunden des Kami bleiben, eine Niederlage setzt beide zurück.
  - Niederlage schickt ins Dorf und entscheidet nichts; der Hain bleibt bewacht.
  - Sieg setzt `OUT_FIGHT`, öffnet den Hain (`guarded` gilt nur ohne Ausgang) und ändert Sumis Dialog über das neue Feld `outcome` in der Regeltabelle.
  - Sieg und Niederlage erscheinen im Textfenster (mehrzeilig), nicht in der einzeiligen Fußzeile.
  - `journey_fight` ist ein eigener Journey-Test; `--verify` spielt beide.

## Stage 3C: Lösung „Alte Grenze wiederherstellen“
**Goal**: Räumliches Rätsel, dessen Bedeutung aus der Geschichte des Ortes folgt.
**Success Criteria**:
- Drei Grenzsteine markieren die alte Vereinbarung zwischen Dorf und Hain. Die Holzfäller haben einen versetzt.
- Beobachtungen `STONE_DRAGGED` (Schleifspur) und `STONE_HOLLOW` (leere Mulde mit Moosrand) schalten „Stein schieben“ frei. Vorher ist der Stein nur ein Stein.
- Schiebe-Mechanik (Sokoban-artig, 1 Tile pro Schub) im Wald. Der Stein in der Mulde → Ausgang `OUT_BOUNDARY`.
- Stein verklemmt → Zurücksetzen per Untersuchen („Du rollst den Stein zurück zur Schleifspur.“).
**Tests**:
- Unit: Schieben ohne Freischaltung wirkungslos; Kollision beim Schieben; Zurücksetzen; Ausgang nur in der Mulde.
- `journey_boundary` grün; alle bisherigen Journeys grün.
**Status**: Complete (2026-09-18).
- Build ohne Warnungen (`grep -c warning` = 0), `ctest` 2/2, `--verify --log` mit Exit 0, Screenshots angesehen.
- Probeweise eingebaute Fehler (Schieben ohne Beobachtungen, Schieben ohne Zielprüfung) werden erkannt.
- Umsetzung:
  - Der versetzte Stein steht nicht mehr in der Karte, sondern ist Spielzustand (`stone_x/stone_y`); `game_tile` blendet ihn ein. Deshalb gibt es die neue Punktart `POINT_STONE`, die ihn überall untersuchbar macht.
  - Schieben ist erst mit `STONE_DRAGGED` **und** `STONE_HOLLOW` möglich. Vorher ist der Stein nur ein Hindernis; kein Text erklärt die Verbindung.
  - Ein verklemmter Stein wird beim Untersuchen an den Anfang der Schleifspur zurückgerollt.
  - In der Mulde: `OUT_BOUNDARY`, Kami wird RUHIG, Hain offen, Sumi und Daigo bewerten es gegensätzlich (Gewinn und Verlust).
  - `journey_boundary` als dritter Journey-Test; `--verify` spielt alle drei.
  - Neues Log-Ereignis: `stone_push` mit der Position des Steins.

## Stage 3D: Kintsugi – von den Scherben zur geflickten Schale
**Goal**: Reparatur als bedeutungsvoller Vorgang, nicht als Rezept. Diese Stufe endet mit einer fertigen, darbringbaren Schale, noch ohne Ausgang.
**Success Criteria**:
- Kette laut Spec: Scherben finden → Hausmarke auf dem Boden der Schale mit der Marke an Sumis Tür verknüpfen (Beobachtung `BOWL_OWNER`) → Oriha gibt Goldlack erst, wenn sie die Geschichte der Schale kennt → **Reparaturansicht** `GAME_MEND`.
- **Reparaturansicht** (ca. 20–40 s): 4 Scherben nacheinander an ihre Bruchkanten setzen. Eine falsche Scherbe passt sichtbar nicht, ohne Strafe. Jede gesetzte Naht wird golden.
- **Trocknen als sichtbarer Weltzustand**, ohne unsichtbaren Zähler: Oriha stellt die Schale sichtbar auf ihr Regal und sagt: „Der Lack braucht Ruhe. Komm wieder, wenn du aus dem Wald zurück bist.“ Beim nächsten Übergang Wald → Dorf wechselt der Zustand `BOWL_DRYING` zu `BOWL_READY`. Danach übergibt Oriha die Schale.
- Die geflickte Schale hat ein eigenes Sprite mit sichtbaren Goldnähten (Regal und Inventar).
- Darbringen der geflickten Schale am Schrein → Kami `RUHIG`. Noch kein Ausgang.
**Tests**:
- Unit: Goldlack nicht ohne `BOWL_OWNER`; Reparatur nicht doppelt; falsche Scherbe ändert nichts; `BOWL_DRYING` → `BOWL_READY` nur beim Übergang Wald → Dorf, nicht bei Dorf → Wald oder durch Herumlaufen im Dorf; Darbringen erst mit `BOWL_READY`.
- Journey: Scherben → Reparatur → Wald → Dorf → Schale → Darbringen → Kami `RUHIG`; alle bisherigen Journeys grün.
**Status**: Complete (2026-09-18).
- Build ohne Warnungen, `ctest` 2/2, `--verify --log` mit Exit 0, Screenshots angesehen.
- Umsetzung:
  - Orihas Angebot führt über das neue Regelfeld `opens` direkt in die Reparaturansicht (`GAME_MEND`).
  - Die Ansicht nennt die offene Bruchstelle und listet die Stücke; ein falsches Stück kostet nichts, die Arbeit lässt sich unterbrechen und fortsetzen.
  - Trocknen ohne Zähler: Die Schale steht sichtbar in Orihas Regal (Tile-Overrides `b`/`q`), und der Wechsel geschieht beim Übergang Wald → Dorf.
  - Die geflickte Schale ist ein eigener Gegenstand und beruhigt den Kami (Gemüt RUHIG); kein Ausgang.
  - `journey_mend` als vierter Journey-Test; neues Log-Ereignis `mend`.
- Abweichung von der Spec: Die Schale wird dem Kami in der Begegnung dargebracht, nicht am Opferstein. Das Gemüt gehört zur Begegnung, und der Opferstein liegt weit vom Hain entfernt. Die Spec ist entsprechend angepasst.

## Stage 3E: Kompromiss mit Daigo
**Goal**: Aus der Beruhigung wird eine Aushandlung zwischen Dorf und Hain.
**Success Criteria**:
- Mit Kami `RUHIG` + `TRACKS` + `LEDGER_DEBT` bietet Daigo an, eine neue Grenze abzustecken.
- Daigo folgt dem Spieler (einfaches Nachlaufen: Daigo übernimmt die vorige Position der Spielfigur). An 3 Pfahl-Stellen entlang der Tierspuren setzt Enter einen Pfahl → Ausgang `OUT_MEND`.
- Verlässt die Spielfigur den Wald, bleibt Daigo am Lager stehen. Gesetzte Pfähle bleiben erhalten.
**Tests**:
- Unit: Angebot nur mit allen drei Bedingungen; Nachlaufen blockiert nicht und durchquert keine Wände; Pfahl nur an den vorgesehenen Stellen und nur mit Daigo in der Nähe; Ausgang erst beim dritten Pfahl.
- `journey_mend` grün; alle bisherigen Journeys grün.
- Test: Kein Ausgang ist ohne seine Voraussetzungen erreichbar (alle drei Wege).
**Status**: Complete (2026-09-18).
- Build ohne Warnungen (vollständig neu übersetzt), `ctest` 2/2, `--verify --log` und `--smoke` mit Exit 0, Screenshots angesehen.
- Probeweise eingebaute Fehler (Pfahl ohne Daigo, Nachlaufen abgeschaltet) werden erkannt.
- **Nachtrag aus dem Review von 3C–3E (2026-09-18):** Behoben: fehlende Tile-Definition für den Grenzpfahl (die Änderung war in einem abgebrochenen Skriptlauf verloren gegangen); `reset_stone` versetzte versehentlich Daigo; Daigo blockierte den Rückweg (Folgende tauschen jetzt den Platz) und blieb nach dem letzten Pfahl mitten im Wald stehen; Sumi bot nach einem Ausgang noch den veralteten Auftrag an (neues `OUT_ANY`, `OUT_NONE` heißt jetzt „noch nichts entschieden“); Oriha sprach nach der Übergabe weiter vom Trocknen; die Meldung der letzten Kampfrunde blieb in der Fußzeile stehen; fehlendes `break` bei der Niederlage. Neue Tests: Pfahl-Orte, Reihenfolge der Scherbenstücke, Erreichbarkeit je Overlay-Bedingung, Platztausch, leere Fußzeile.
- Umsetzung:
  - Daigos Angebot kommt erst mit ruhigem Kami, bekannten Fährten und gelesenem Auftragsbuch; das Regelfeld `opens` startet das Nachlaufen.
  - Er übernimmt jeweils das zuletzt verlassene Feld der Spielfigur und blockiert nie den Weg. Verlässt man den Wald, kehrt er ans Lager zurück.
  - Ein Pfahl geht nur an den drei Stellen entlang der Fährten und nur mit Daigo daneben.
  - Der dritte Pfahl setzt `OUT_MEND`; Sumi und Daigo bewerten es unterschiedlich (weniger Holz, jährliche Gabe, dafür Ruhe).
  - `journey_mend` führt jetzt bis zum Ausgang; neues Log-Ereignis `stake`.
- Anmerkung: Das Protokoll hatte den Pfahl-Fall zwischenzeitlich nicht geschrieben (verlorene Änderung); behoben und mit vollständigem Neubau geprüft.

## Stage 4: Konsequenzen, Zeitfortschritt, Leerer-Gast-Hook
**Goal**: Jeder Ausgang verändert die Welt sofort und am nächsten Morgen, mit Gewinn und Verlust. Eine Spur weckt Neugier, ohne zu erklären.
**Success Criteria**:
- Tile-Overrides, NPC-Dialoge, Tieranwesenheit und Zugänge gemäß der Konsequenzmatrix der Spec.
- **Klang**: minimale SDL3-Audioausgabe (Emberpost hat noch kein Audio), 2–3 Ambient-Loops, je Ausgang/Ort umgeschaltet.
- **Zeitfortschritt**: Übernachtung im Gasthaus → `PHASE_MORNING` mit zweiter Override-Welle.
- **Hook**: Nach jedem Ausgang liegt am alten Schrein eine graue Spur. Untersuchen ergibt: „Keine Fußspur. Nichts Verbranntes. Keine Flechte.“ Mit Tierzeichen: „Nicht einmal Tiere sind hier gegangen.“ Die Inschrift nennt einen Besucher, „bevor der Wald unruhig wurde“. Abspann-Teaser ohne Namen.
- **Balance-Guard** (Designregel 3): Jeder Ausgang hat in der Datentabelle pro Phase mindestens eine Änderung mit Tag `GAIN` und eine mit Tag `LOSS`. Das ist nur ein **formaler** Schutz gegen vergessene Verluste. Ob sich ein Ende besser anfühlt, zeigen erst die Playtest-Fragen 3 und 4.
**Tests**:
- Unit: Overrides je Ausgang/Phase; Konnektivität der Karten in jedem Zustand; Balance-Guard.
- Journeys aller drei Ausgänge inkl. Übernachtung und Hook.
- `--verify`: Screenshot-Serie je Ausgang (Dorf, Hain, Morgen, Schrein-Spur) angesehen.
**Status**: Complete (2026-09-18).
- Build ohne Warnungen, `ctest` 2/2, `--verify --log` und `--smoke` mit Exit 0, Screenshots angesehen (Morgen je Ausgang am Holzplatz).
- Umsetzung:
  - `TileOverride` und Dialogregeln kennen jetzt Ausgang **und** Phase (`PHASE_ANY`/`OUT_ANY` heißt „egal“).
  - Die Konsequenzen stehen als eigene Tabelle `outcome_changes` mit Tag `GAIN`/`LOSS`.
  - Übernachtung auf dem Futon im Gasthaus, aber erst nach einer Entscheidung; sonst: „Du liegst wach.“
  - Morgen-Dialoge für Sumi, Mio und Daigo je Ausgang.
  - Graue Spur am Schrein nach jedem Ausgang; mit Tierzeichen zusätzlich „nicht einmal Tiere“. Erst danach zeigt die Inschrift die Zeile über den Besucher, gefolgt vom Abspann-Teaser.
  - Klang: zweistimmige Ambient-Fläche mit Rauschanteil, je nach Ort und Ausgang; ohne Audiogerät läuft das Spiel weiter.
- **Nachtrag (2026-09-19):** Der Übergang in den Morgen wurde verständlicher gemacht.
  - Nach dem Gespräch mit Sumi fragt sie, ob im Gasthaus übernachtet werden soll („Übernachten“ / „Noch hierbleiben“). Ablehnen ist folgenlos; die Nacht bleibt bei Sumi und am Futon verfügbar.
  - Beide Wege nutzen dieselbe Funktion `spend_the_night`; danach liegt die Figur im Gasthaus.
  - Der Futon reagiert jetzt auch vom davorliegenden Feld. Nach einer Lösung erscheint dort kein Hinweis mehr auf einen unruhigen Wald, weil der Übergang vor den Untersuchungspunkten greift.
  - Vor einer Lösung bleibt die Sperre; der Morgenwechsel geschieht weiterhin nur einmal.
- **Selbstprüfung statt Review (2026-09-19):** Das angeforderte Review brach wegen eines Sitzungslimits ab. Eigene Prüfung fand zwei Fehler: Konsequenzen wurden von älteren Overlays verdeckt (Fuchsbau blieb nach dem Kampf „bewohnt“), und Sumis Morgen-Regeln verdeckten die Schalen-Geschichte. Beides behoben und mit Tests abgesichert. Ein unabhängiges Review von Stage 4 steht noch aus.
- Tests: Prüfung auf Gewinn und Verlust je Ausgang **und** Phase, Sichtbarkeit jeder Änderung, Begehbarkeit in jedem Zustand, Übernachtung nur nach Entscheidung, zweite Nacht ohne Wirkung, Spur und späte Inschrift. Alle drei Journeys laufen bis zum Abspann.

## Stage 5A: Interner Playthrough & Kulturcheck
**Goal**: Der Slice ist in rund 30 Minuten ohne Vorwissen durchspielbar. Die Inhalte sind kulturell geprüft, bevor Fremde spielen.
**Success Criteria**:
- Rainer spielt alle drei Wege mit `--log`, Zeit und Stolperstellen werden dokumentiert.
- Atlas nur so weit ergänzt, wie es die Lesbarkeit der Hinweise erfordert (Log `examine_nothing` auswerten).
- Tempo-Kontrast vorhanden: Humor (Kenta), Gefahr (Rückstoß), Rätsel (Stein, Scherben).
- `docs/culture-notes.md`: Checkliste (Schrein, Shimenawa, Kitsune/Inari-Bezug, Opfergaben, keine Riten als Magie) mit Fachquellen; Sensitivity-Review angefragt.
- `docs/playtest-protocol.md` fertig.
**Tests**:
- Alle Unit- und Journey-Tests grün.
- Dokumentierter interner Durchlauf je Ausgang.
**Status**: Not Started

## Stage 5B: Externe Playtests (3–5 Personen)
**Goal**: Die Kernhypothese mit echten Spielenden prüfen.
**Success Criteria**:
- Durchführung nach Protokoll (siehe unten), Log und Interview je Person.
- Auswertung in `docs/playtest-results.md` mit Go/No-Go für Stage 6.
**Tests**:
- Jede Session hat Log, Interview-Notizen und Zeitmessung.
**Status**: Not Started

## Stage 6 (nur bei positivem Ergebnis): Politur, Windows, C64U-Evaluierung
**Goal**: Den validierten Slice vorzeigbar machen und die nächste Entscheidung vorbereiten.
**Success Criteria**:
- Überarbeitung von Grafik und Klang.
- Windows-Paket (MinGW, statisch gelinkt) wie bei Emberpost; README, THIRD_PARTY und `docs/validation.txt` aktualisiert.
- Kurze C64U-Evaluierung: Speicherbedarf der Tabellen und Karten, was portierbar ist.
**Tests**:
- Tests grün unter Linux und Windows; `--verify` im Windows-Paket mit Exit 0.
**Status**: Not Started

---

## Playtest & Messung

Gemessen wird **Hypothesenbildung und Verhalten**, nicht Vollständigkeit.

**Ablauf je Session** (ca. 45 min): keine Einführung außer der Steuerung, lautes
Denken erwünscht, die Beobachtung bleibt still.

1. Nach dem Auftrag: „Was ist deine Aufgabe? Was wirst du tun?“
2. **Vor der ersten Kami-Begegnung** (die Spielleitung pausiert beim ersten Betreten
   des Hainrands): „Was glaubst du, passiert hier?“ und „Wie sicher bist du (1–5)?“
3. **Nach der Begegnung / vor der Entscheidung**: „Welche Möglichkeiten glaubst du
   zu haben?“ und „Warum wählst du diese?“
4. Nach dem Morgen: „Was hat sich verändert? Was hast du gewonnen, was verloren?“
5. Zum Schluss: „Was, glaubst du, war am Schrein?“ und „Würdest du die nächste Region spielen? Warum?“

| Konzept-Kriterium | Auswertung |
|---|---|
| Erkundung motiviert zum Suchen | Log: Welche Beobachtungen, in welcher Reihenfolge, mit welchen Pausen; wie viele freiwillige Untersuchungen (inkl. `examine_nothing`) |
| Lösung aus Beobachtung ableitbar | Frage 2: Hypothese vorhanden? Welche Beobachtungen nennt die Person? Log: Zeitpunkt der Hypothese im Verhältnis zu den gefundenen Beobachtungen. Früh verstanden und weniger gesucht ist ein **gutes** Ergebnis. |
| ≥3 Lösungswege mechanisch verschieden | Journey-Tests + Frage 3: Wie viele Wege erkennt die Person? |
| Keine Lösung moralisch „richtig“ | Frage 3/4: Wird die Wahl als Abwägung begründet? Werden Verluste genannt? Streuung der Wahl über die Personen |
| Konsequenz sofort und später sichtbar | Frage 4: beide Phasen unaufgefordert genannt? |
| Leerer Gast weckt Neugier | Frage 5: eigene Deutungen, Wunsch weiterzuspielen |

## Risiken & Gegenmaßnahmen

- **Zu viele Systeme**: nur Beobachtungen, Dialogregeln, Freischaltungen, Begegnung, Overrides, eine Wahrnehmungsfähigkeit und eine Reparaturansicht.
- **Dialogketten statt Gameplay**: Mindestens 70 % der Beobachtungen kommen aus der Umgebung.
- **Rätsel als Adventure-Rezept**: Jede Handlung braucht eine im Spiel erzählte Bedeutung (Designregel 4). Das wird im Spec-Review geprüft.
- **Kompromiss als heimliches gutes Ende**: Der Balance-Guard fängt nur vergessene Verluste ab. Maßgeblich sind die Fragen 3/4 im Playtest und die Streuung der Wahl.
- **30 Minuten nicht erreicht**: Hinweise verdichten statt Karten vergrößern.
- **Große Änderungssätze durch Agenten**: Kleine Stufen, jede mit eigenem Journey-Test.
- **Kulturelle Oberflächlichkeit**: fiktive Namen und Riten, Prüfung vor 5B.

## Entschieden

- Projektort: `~/projekte/own/cpp/vergessene-pfade`.
- Bezeichner im Code auf Englisch, Spieltexte auf Deutsch in ASCII-Umschrift.
- Pflicht-Ausgänge: Bekämpfen, Alte Grenze wiederherstellen, Kintsugi & Kompromiss. Weitere Wege (z. B. Opfergabe ohne Reparatur) nur, wenn der Playtest es nahelegt.
- Die Zielgruppenfrage (Kampfanteil) wird nach 5B neu bewertet.
