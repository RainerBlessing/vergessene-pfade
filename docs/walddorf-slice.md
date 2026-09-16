# Spielbares Design: Vertical Slice „Das Walddorf“

Entwurf zur Abnahme in Stage 1. Alle Namen, Orte und Handlungen sind fiktiv und
japanisch *inspiriert*. Die kulturelle Prüfung erfolgt in Stage 5A. Thematische
Inspiration ist H. Yoda / S. Blind, „Acht Millionen Wege zum Glück“: Kami wohnen
in Natur und Dingen. Das Buch ist Recherchehintergrund, keine Vorlage.

Die Designregeln 1–5 aus `IMPLEMENTATION_PLAN.md` gelten hier durchgehend.

## 1. Motiv → Mechanik

| Motiv | Umsetzung |
|---|---|
| Kami in Natur und Orten | Waldkami mit Gemüt; der Hain als Ort, der verteidigt wird |
| Kintsugi / Wabi-Sabi | Reparaturansicht; Goldnähte bleiben sichtbar, der Kami erkennt die Schale *wegen* ihrer Geschichte |
| Ahnen, Erinnerung | Die Schale gehörte Sumis Großmutter, der letzten Schreinhüterin. Niemand pflegt den Schrein mehr. |
| Vereinbarung Mensch/Ort | drei alte Grenzsteine, Inschrift am Schrein |
| Ikigai / Lebensgrundlage | Daigos Schulden, Kentas Arbeit, das Holz für den Winter |
| Vergessen (Leerer Gast) | Spur ohne Eigenschaften; ein Besucher *vor* der Unruhe |

## 2. Orte

### Dorf „Kiriyama“ (`village.map`, ≈32×24)
- Haus der Dorfältesten Sumi; an der Tür eine **Hausmarke** (untersuchbar)
- Werkstatt der Lackmeisterin Oriha
- Gasthaus (Übernachtung → Morgen-Phase)
- Holzplatz (fast leer: Das Dorf braucht Holz für den Winter)
- Nordtor → Wald

### Wald (`forest.map`, ≈48×40)
```
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  ^        [ Heiliger Hain ]                ^
  ^   G1 ....  ~Baum~  .... G3              ^
  ^   (Rand = Gefahrenzone / Begegnung)     ^
  ^        m   G2'  x x x Stümpfe           ^
  ^     F = Fuchsbau          o = Schrein   ^
  ^  L = Holzfällerlager        ~~~ Bach    ^
  ^        ,,,,,,,,,,,,,,,,,,,,,,,,         ^
  ^^^^^^^^^^^^^^^^^^,^^^^^^^^^^^^^^^^^^^^^^^
                    Dorf
  G1, G3 = Grenzsteine an alter Stelle   G2' = versetzter Stein
  m = leere Mulde (ursprünglicher Platz von G2)
```
Wegezeit Dorf → Hain: etwa 1 Minute. Die Untersuchungspunkte liegen abseits des Hauptwegs.
Der Moosboden des Hains ist vor einer Auflösung bewacht (Rückstoß). Der alte
Baum steht deshalb direkt am Hainrand und ist von der Grenzlinie aus zu untersuchen.

## 3. Figuren

| NPC | Rolle | Haltung |
|---|---|---|
| Dorfälteste **Sumi** | gibt den Auftrag „Vertreibe den Geist“ | pragmatisch; Enkelin der letzten Schreinhüterin, hat den Schrein aufgegeben |
| Vorarbeiter **Daigo** | leitet das Lager, verletzt | zornig, nicht böse; muss einen Holzauftrag erfüllen, sonst verliert das Lager alles |
| **Kenta**, Sumis Sohn, Holzfäller | Humor, Angsthase („Der Baum hat mich *angeschaut*.“) | braucht die Arbeit |
| Lackmeisterin **Oriha** | Kintsugi | „Ich flicke nichts, dessen Geschichte ich nicht kenne.“ |
| Kind **Mio** | kennt den Fuchs, hat Heilkraut | reagiert stark auf jeden Ausgang |
| Waldkami (namenlos) | Begegnung | Gemüt wechselt je Handlung |

## 4. Beobachtungen

Nur Beobachtungen, keine Schlussfolgerungen (Designregel 1). U steht für Umgebung, D für Dialog.

| Beobachtung | Fundort | Q | Notizbuch-Text (Entwurf) |
|---|---|---|---|
| `CLAW_MARKS_EDGE` | Kratzspuren an G1/G3 | U | „Die Kratzspuren enden an den alten Steinen. Dahinter keine.“ |
| `FRESH_STUMPS` | Stümpfe am Hainrand | U | „Frisches Harz. Diese Bäume fielen vor wenigen Tagen.“ |
| `BROKEN_ROPE` | zerrissenes Seil am Hainbaum (vom Rand aus) | U | „Um den großen Baum hing ein Seil. Es ist gerissen, nicht verrottet.“ |
| `SHRINE_INSCRIPTION` | Schrein | U | „Eingeritzt: ‚Bis zu den drei Steinen und nicht weiter.‘“ |
| `STONE_DRAGGED` | Schleifspur an G2' | U | „Eine Schleifspur. Jemand hat diesen Stein bewegt.“ |
| `STONE_HOLLOW` | Mulde m | U | „Eine Mulde mit Moosrand. Hier lag lange etwas Schweres.“ |
| `BOWL_SHARDS` (+ Gegenstand) | Schrein | U | „Scherben einer Schale. In einer liegen vertrocknete Beeren.“ |
| `BOWL_MARK` | Boden einer Scherbe | U | „Auf dem Boden ein eingebranntes Zeichen.“ |
| `HOUSE_MARK` | Sumis Tür | U | „Dasselbe Zeichen wie auf der Schale?“ erst mit `BOWL_MARK`, sonst: „Ein eingebranntes Hauszeichen.“ |
| `BOWL_OWNER` | Sumi ansprechen mit `BOWL_MARK`+`HOUSE_MARK` | D | „Die Schale gehörte Sumis Großmutter.“ |
| `LEDGER_DEBT` | Auftragsbuch im Lager | U | „Ein Holzauftrag aus der Stadt. Die Frist ist in zehn Tagen.“ |
| `FOX_WOUNDED` | Fuchsbau | U | „Der Fuchs ist verletzt. Seine Pfote trägt dunklen Lehm.“ |
| `KENTA_STARE` | Kenta | D | „Kenta sagt, der Baum habe ihn angesehen.“ |
| `TRACKS` | Fährten (nur mit Tierzeichen) | U | „Tierspuren meiden das Lager und laufen in einem Bogen um den Hain.“ |
| `GREY_TRACE` | Schrein, nur nach einem Ausgang | U | „Keine Fußspur. Nichts Verbranntes. Keine Flechte.“ |

Die Fähigkeit **Tierzeichen** erhält man, indem man den Fuchs mit Mios Heilkraut versorgt.

## 5. Freischaltungen (Beobachtungen → Handlungen)

| Handlung | benötigt | Wirkung |
|---|---|---|
| Kami angreifen | – | Kampf (3B) |
| Stein G2' schieben | `STONE_DRAGGED` + `STONE_HOLLOW` | Schiebe-Rätsel (3C) |
| Oriha um Goldlack bitten | `BOWL_SHARDS` + `BOWL_OWNER` | Reparaturansicht (3D) |
| geflickte Schale darbringen | `BOWL_READY` (nach Rückkehr aus dem Wald abgeholt) | Kami → RUHIG |
| Daigo: neue Grenze vorschlagen | Kami RUHIG + `TRACKS` + `LEDGER_DEBT` | Daigo folgt zu den Pfählen (3E) |
| Scherben darbringen (ungeflickt) | `BOWL_SHARDS` | Kami → WUETEND (Fehlgriff mit Folge) |

## 6. Begegnung (Gemüt)

| Aktion \ Gemüt | WUETEND | MISSTRAUISCH | RUHIG |
|---|---|---|---|
| Angreifen | WUETEND (Kampf) | WUETEND (Kampf) | WUETEND (Kampf) |
| Warten | MISSTRAUISCH | MISSTRAUISCH | RUHIG |
| Zurückweichen | Begegnung endet | Begegnung endet | Begegnung endet |
| geflickte Schale darbringen | MISSTRAUISCH | RUHIG | RUHIG |
| Scherben darbringen | WUETEND | WUETEND | MISSTRAUISCH |

## 7. Die drei Ausgänge

### Bekämpfen (`OUT_FIGHT`)
Rundenbasierter Kampf; im Zustand WUETEND schlägt der Kami härter zu.

### Alte Grenze wiederherstellen (`OUT_BOUNDARY`)
Den versetzten Stein G2' zurück in die Mulde schieben (Sokoban-artig, 1 Tile pro
Schub). Die Bedeutung ergibt sich aus der Inschrift. Wer Stein und Mulde
beobachtet hat, *kann* schieben, muss aber selbst die Verbindung ziehen. Ein
verklemmter Stein lässt sich zurücksetzen.

### Kintsugi & Kompromiss (`OUT_MEND`)
```
Scherben finden → Zeichen am Boden bemerken → dasselbe Zeichen an Sumis Tür
  → Sumi erzählt von ihrer Großmutter → Oriha gibt Goldlack
  → Reparaturansicht: 4 Scherben an ihre Bruchkanten setzen, Nähte werden golden
  → Oriha stellt die Schale sichtbar aufs Regal: „Komm wieder, wenn du aus dem Wald zurück bist.“
  → beim nächsten Übergang Wald → Dorf ist sie fertig (BOWL_DRYING → BOWL_READY)
  → geflickte Schale am Schrein darbringen → Kami RUHIG
  → mit Daigo eine neue Grenze entlang der Tierspuren abstecken (3 Pfähle)
```
**Reparaturansicht**: Die Schale steht als Umriss in der Mitte, die 4 Scherben
liegen daneben. Die Person wählt eine Scherbe und eine Bruchstelle. Passt sie
nicht, dreht sich die Scherbe kurz zurück; das kostet nichts. Jede gesetzte Naht
glänzt golden. Oriha: „Der Bruch gehört jetzt zur Schale.“

**Trocknen**: Es gibt keinen unsichtbaren Zähler. Die Schale steht sichtbar im
Regal der Werkstatt, und der Wechsel geschieht an einem klaren Weltereignis
(Rückkehr aus dem Wald). Im Dorf im Kreis zu laufen bringt nichts. Die Umsetzung
ist auf zwei Stufen verteilt: 3D bis zur beruhigten Begegnung, 3E für die
Aushandlung mit Daigo.

## 8. Konsequenzmatrix (Gewinn und Verlust)

| Ausgang | Phase | Gewinn (`GAIN`) | Verlust (`LOSS`) |
|---|---|---|---|
| Bekämpfen | sofort | Lager arbeitet, Holzplatz füllt sich, neuer Holzweg nach Osten | Kami fort, Hain gerodet (Stümpfe), Vögel verstummen |
| | Morgen | Daigo zahlt Schulden, Kenta hat Arbeit, das Dorf feiert | Bach trüb, Fuchs verschwunden, Mio meidet die Spielfigur |
| Grenze wiederherstellen | sofort | Hain unberührt, Kami zieht sich zurück, Angriffe enden | Lager verliert die besten Bäume, Daigo ist feindselig |
| | Morgen | Vögel kehren zurück, Sumi öffnet den Schrein wieder | Lager schließt; Kenta ohne Arbeit, Holzplatz leer; Sumi: „Die Steine stehen wieder. Mein Sohn hat keine Arbeit.“ |
| Kintsugi & Kompromiss | sofort | Kami bleibt, Hainpfad öffnet sich, Totholzstapel | Der Hainrand mit alten Bäumen bleibt verloren, Daigo murrt |
| | Morgen | Setzlinge, Fuchs mit Jungen, der Schrein wird gepflegt | Zu wenig Holz für den Auftrag, zwei Familien verlieren Einkommen; das Dorf muss dem Hain jedes Jahr eine Gabe bringen |

Zu jedem Ausgang gibt es eigene Klang-Loops, Dialoge aller NPCs und die Anwesenheit von Tieren.

## 9. Ablauf (≈30 min)

1. **Ankunft** (2 min): Kenta flieht aus dem Wald, die Spielfigur wird am Hainrand zurückgestoßen.
2. **Auftrag** (3 min): Sumi: „Vertreibe den Geist.“
3. **Erkundung** (12–15 min): Lager, Stümpfe, Grenzsteine, Schrein, Fuchs, Dorf.
4. **Verstehen**: Das Spiel sagt nichts dazu. Die Person bildet eine Hypothese; das Interview fragt sie vor der Begegnung ab.
5. **Handeln** (5 min): Kampf, Stein oder Schale und Grenze.
6. **Konsequenz** (3 min): Veränderungen; Übernachtung → Morgen.
7. **Hook** (2 min): graue Spur am Schrein; Inschrift: „… kam ein Besucher, bevor der Wald unruhig wurde.“ Danach kurzer Abspann.

## 10. Datenmodell (Core, SDL-frei)

```c
typedef uint32_t Obs;                   /* Bit je Beobachtung, keine Schlussfolgerungen */
typedef enum { OUT_NONE, OUT_FIGHT, OUT_BOUNDARY, OUT_MEND } Outcome;
typedef enum { PHASE_BEFORE, PHASE_AFTER, PHASE_MORNING } Phase;
typedef enum { MOOD_ANGRY, MOOD_WARY, MOOD_CALM } Mood;
typedef enum { TAG_NEUTRAL, TAG_GAIN, TAG_LOSS } ChangeTag;

typedef struct { uint8_t map, x, y; Obs requires; uint8_t ability; uint8_t obs, text; } ExaminePoint;
typedef struct { uint8_t npc; Obs requires, forbids; Outcome outcome; Phase phase; uint8_t dialogue; } DialogueRule;
typedef struct { uint8_t action, target; Obs requires; uint8_t item; } Unlock;
typedef struct { Outcome outcome; Phase phase; uint8_t map, x, y; char symbol; ChangeTag tag; } TileOverride;
```
Die Tabellen werden in Reihenfolge ausgewertet, die erste Übereinstimmung gewinnt.
Es gibt keine Sonderfälle pro NPC im Code.
