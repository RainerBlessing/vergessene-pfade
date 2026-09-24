# Spezifikation: Driftmelder für die Inhalte (PC ↔ C64)

## Warum

`src/content.c` (SDL/PC) und `c64/src/content.c` (C64) pflegen dieselben Texte
doppelt. Der C64 hat bewusst nur einen Teil davon. Heute weichen von 64
gemeinsamen Dialogen genau **einer** und von 28 gemeinsamen Notizen **keine** ab.
Das Skript soll verhindern, dass daraus unbemerkt mehr wird: Abweichungen sollen
nicht unmöglich sein, sondern **eine Entscheidung erzwingen**.

## Was gebaut wird

1. `tools/content_drift.py` — Prüfskript, Python 3, **nur Standardbibliothek**,
   kein Netz, deterministisch, Laufzeit unter 2 s. Es **verändert keine Datei**.
2. `tools/content_drift_allowlist.json` — Liste bewusster Abweichungen.
3. `tools/test_content_drift.py` — Tests, lauffähig mit
   `python3 -m unittest tools.test_content_drift` oder direkt.
4. Ein Schritt in `.github/workflows/c64.yml`, der das Skript aufruft
   (Name des Schritts: „Inhalte vergleichen“), eingefügt **vor** „PRG bauen“.

Sonst wird nichts geändert: keine Änderung an `src/`, `c64/src/`, `assets/`,
keine neuen Abhängigkeiten, keine Umbauten am Build.

## Was verglichen wird

Aus beiden Dateien werden die Zeichenketten-Tabellen gelesen:

| Tabelle | Schlüssel | Vergleichswert |
|---|---|---|
| `dialogues[]` | `D_*` | Seitenzahl und Text je Seite |
| `notes[]` | `N_*`, `NOTE_NONE` | Text |
| `npcs[]` | Reihenfolge im Feld (Index) | Anzeigename (letztes Feld) |
| `items[]` | Index | Name |
| `mood_names[]` | Index | Text |

Verglichen wird **nur, was es in beiden gibt**. IDs, die nur eine Seite kennt,
sind kein Befund — der C64 hat absichtlich weniger. Bei `npcs`, `items` und
`mood_names` wird über den gemeinsamen Indexbereich verglichen.

### Normalisierung

- Aneinandergereihte C-Zeichenkettenliterale (`"a" "b"`) werden zu einem Text
  verbunden, bevor verglichen wird. Wo die Quelltextzeilen umbrechen, ist egal.
- `\n` im Text ist **Zeilenumbruch im Spiel** und damit Anzeige, nicht Inhalt:
  vor dem Vergleich wird `\n` zu einem Leerzeichen, und Folgen von Leerzeichen
  werden zu einem. Der PC bricht bei 36 Zeichen um, der C64 bei 38 — dieser
  Unterschied darf keinen Befund erzeugen.
- Die **Seitenzahl** wird ohne Normalisierung verglichen und getrennt gemeldet:
  Ein zweiseitiger Text, der einseitig wurde, ist ein Befund eigener Art.

## Ausnahmeliste

`tools/content_drift_allowlist.json`, Form:

```json
{
  "dialogues": {
    "D_ENC_OFFER_BOWL": "Die Begegnungstafel auf dem C64 blättert nicht: RETURN führt die Handlung aus. Der Text muss dort auf eine Seite passen."
  },
  "notes": {},
  "npcs": {},
  "items": {},
  "mood_names": {}
}
```

- Jeder Eintrag braucht einen **Grund** als nichtleeren Text. Ein Eintrag ohne
  Grund ist ein Befund.
- Ein Eintrag, dessen Abweichung es **nicht mehr gibt**, ist ebenfalls ein Befund
  („veralteter Eintrag, bitte entfernen“) — die Liste soll nicht verrotten.
- Beim ersten Lauf muss genau `D_ENC_OFFER_BOWL` in der Liste stehen, mit dem
  oben genannten Grund. Danach muss das Skript grün durchlaufen.

## Ausgabe und Rückgabewert

- Ohne Befund: eine kurze Zeile je verglichener Tabelle mit der Zahl der
  gemeinsamen Einträge, Rückgabewert **0**.
- Mit Befund: je Befund die ID, die Art (Text / Seitenzahl / fehlender Grund /
  veralteter Eintrag) und **beide Fassungen gekürzt auf je 100 Zeichen**,
  Rückgabewert **1**.
- Ausgabe auf Deutsch, ohne Farben, ohne Fortschrittsbalken.
- `--allowlist PFAD` erlaubt eine andere Liste (für die Tests).
- `--help` erklärt den Zweck in zwei Sätzen.

## Tests

`tools/test_content_drift.py` prüft mindestens:

1. Gleicher Text in beiden → kein Befund.
2. Unterschiedlicher Text → ein Befund, der die ID nennt.
3. Unterschied nur in Zeilenumbrüchen (`\n` gegen Leerzeichen) → kein Befund.
4. ID nur im PC vorhanden → kein Befund.
5. Unterschiedliche Seitenzahl bei sonst gleichem Text → Befund mit der Art
   „Seitenzahl“.
6. Eintrag in der Ausnahmeliste → kein Befund.
7. Ausnahmeliste enthält eine ID, die gar nicht abweicht → Befund
   („veralteter Eintrag“).
8. Ein Lauf gegen die echten Dateien des Repos → kein Befund.

Die Tests dürfen die echten Dateien lesen, aber keine schreiben. Für die Fälle
1–7 werden kleine Quelltextausschnitte im Test selbst gebildet, keine Dateien im
Repo angelegt. Dafür muss das Skript eine Funktion anbieten, die **Text statt
Pfad** entgegennimmt, etwa `vergleiche(pc_quelltext, c64_quelltext, ausnahmen)`.

## Grenzen

- Regeltabellen (`dialogue_rules`, `examine_points`, `tile_overrides`) werden
  **nicht** verglichen. Sie unterscheiden sich im Umfang und in den Typen; das
  wäre ein zweiter Schritt.
- Das Skript erzeugt nichts und repariert nichts. Es meldet.
