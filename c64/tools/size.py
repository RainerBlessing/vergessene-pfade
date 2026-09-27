#!/usr/bin/env python3
"""Speicherbericht fuer den C64-Build: `make size`.

Zeigt, was die .prg belegt (Code, Nur-Lese-Daten, beschreibbare Daten, BSS)
und wer die groessten Verbraucher sind. Die Zahlen kommen aus dem ELF neben
der .prg, gelesen mit llvm-size und llvm-nm aus dem llvm-mos-SDK.

Der Textpool ist die Differenz zwischen .rodata und der Summe der benannten
Symbole darin: die Zeichenketten, die keinen eigenen Namen haben (Dialogseiten,
Notizen, Titel). Ohne diese Zeile sieht .rodata aus wie lauter kleine Tabellen.
"""
import os
import pathlib
import subprocess
import sys

SDK_BIN = pathlib.Path(
    os.environ.get("LLVM_MOS_BIN", pathlib.Path.home() / ".local/share/llvm-mos-sdk/bin"))
TOP = int(os.environ.get("SIZE_TOP", "20"))

# Welche Abschnitte zu welcher Gruppe gehoeren. Alles, was in der .prg liegt,
# kostet Dateigroesse; BSS und noinit kosten nur RAM zur Laufzeit.
IN_PRG = ("Code", "Nur-Lese-Daten", "Daten")
GROUPS = {
    ".text": "Code",
    ".rodata": "Nur-Lese-Daten",
    ".data": "Daten",
    ".zp.data": "Daten",
    ".basic_header": "Daten",
    ".bss": "BSS",
    ".zp.bss": "BSS",
    ".zp": "BSS",
    ".noinit": "BSS",
}
# nm-Typbuchstabe -> Gruppe. Kleinbuchstabe heisst lokal, gross global.
SYM_GROUP = {"t": "Code", "r": "Nur-Lese-Daten", "d": "Daten", "b": "BSS"}


def tool(name, *args):
    path = SDK_BIN / name
    if not path.exists():
        sys.exit(f"{path} fehlt -- llvm-mos-SDK installieren (siehe README) "
                 f"oder LLVM_MOS_BIN setzen")
    return subprocess.run([str(path), *args], check=True, capture_output=True,
                          text=True).stdout


def sections(elf):
    """Abschnittsname -> (Groesse, Adresse), in der Reihenfolge des ELF."""
    out = {}
    for line in tool("llvm-size", "-A", elf).splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[0].startswith(".") and parts[1].isdigit():
            out[parts[0]] = (int(parts[1]), int(parts[2]))
    return out


def symbols(elf):
    """(Gruppe, Groesse, Name) je benanntem Symbol mit Groesse."""
    out = []
    for line in tool("llvm-nm", "--print-size", "--radix=d", elf).splitlines():
        parts = line.split()
        if len(parts) != 4:
            continue  # Symbole ohne Groesse (undefiniert, absolut)
        _addr, size, kind, name = parts
        group = SYM_GROUP.get(kind.lower())
        if group:
            out.append((group, int(size), name))
    return out


def bar(part, whole, width=24):
    filled = round(width * part / whole) if whole else 0
    return "#" * filled + "." * (width - filled)


def main():
    prg = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                       else "build/vergessene-pfade.prg")
    elf = prg.with_suffix(".prg.elf") if prg.suffix == ".prg" else prg
    if not elf.exists():
        sys.exit(f"{elf} fehlt -- erst `make build`")

    secs = sections(str(elf))
    syms = symbols(str(elf))

    totals = {}
    for name, (size, _addr) in secs.items():
        group = GROUPS.get(name)
        if group:
            totals[group] = totals.get(group, 0) + size
    prg_total = sum(totals.get(g, 0) for g in IN_PRG)
    ram_total = prg_total + totals.get("BSS", 0)

    print("Speicher: build/vergessene-pfade.prg")
    print()
    print(f"  {'Datei (mit Ladeadresse)':<24}{prg.stat().st_size:>7}")
    print(f"  {'RAM belegt':<24}{ram_total:>7}  ({ram_total * 100 // 65536}% von 64K)")
    print()
    print(f"  {'Gruppe':<24}{'Bytes':>7}  {'Anteil an der .prg':<24}")
    for group in ("Code", "Nur-Lese-Daten", "Daten", "BSS"):
        size = totals.get(group, 0)
        share = "" if group == "BSS" else bar(size, prg_total)
        print(f"  {group:<24}{size:>7}  {share:<24}")
    print()
    print("  Abschnitte im Einzelnen")
    for name, (size, addr) in secs.items():
        if name in GROUPS:
            print(f"    {name:<22}{size:>7}  ${addr:04x}")

    # Was in .rodata keinen Namen hat, sind die Zeichenketten.
    named_rodata = sum(s for g, s, _ in syms if g == "Nur-Lese-Daten")
    rodata = secs.get(".rodata", (0, 0))[0]
    pool = rodata - named_rodata
    print()
    print("  Nur-Lese-Daten aufgeteilt")
    print(f"    {'benannte Tabellen':<22}{named_rodata:>7}")
    print(f"    {'Textpool (Zeichenketten)':<22}{pool:>7}")

    print()
    print(f"  Groesste Symbole (Top {TOP})")
    print(f"    {'Bytes':>7}  {'Gruppe':<16}Symbol")
    for group, size, name in sorted(syms, key=lambda s: -s[1])[:TOP]:
        print(f"    {size:>7}  {group:<16}{name}")


if __name__ == "__main__":
    main()
