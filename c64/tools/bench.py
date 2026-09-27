#!/usr/bin/env python3
"""Takte je Bildaufbau: `make bench`.

Startet build/bench.prg in x64sc (Warp, ohne Ton), wartet, bis das Programm
gemessen hat, und holt das Ergebnis ueber den VICE-Monitor aus dem RAM. Die
Adressen der Zaehler kommen aus dem ELF neben der .prg, gelesen mit llvm-nm --
wie in size.py.

Braucht x64sc und eine Anzeige (DISPLAY), weil VICE mit GTK gebaut ist; ohne
Anzeige bricht der Lauf mit einer Meldung ab, statt stumm zu haengen. Darum
laeuft `make bench` nicht in CI.

Eine offene Monitor-Verbindung haelt die emulierte CPU an, `x` gibt sie wieder
frei und schliesst dabei die Verbindung -- deshalb wird zum Nachsehen jedes Mal
neu verbunden.
"""
import os
import pathlib
import re
import socket
import subprocess
import sys
import time

SDK_BIN = pathlib.Path(
    os.environ.get("LLVM_MOS_BIN", pathlib.Path.home() / ".local/share/llvm-mos-sdk/bin"))
PORT = int(os.environ.get("BENCH_PORT", "6510"))
READY = 0x42
# Sicherheitsnetz: nach so vielen emulierten Takten beendet sich x64sc selbst,
# damit kein Emulator stehen bleibt, wenn hier etwas schiefgeht. Im Warp sind
# das rund 40 Sekunden echte Zeit -- viel mehr, als Laden und Messen brauchen.
# Gemessen wird in Takten, nicht in echter Zeit; Warp verfaelscht nichts.
LIMIT_CYCLES = 2_000_000_000
WAIT_FIRST = 4.0   # Sekunden: solange braucht Laden und Messen im Warp
TRIES = 20


def symbols(elf):
    """Symbolname -> Adresse, aus dem ELF neben der .prg."""
    path = SDK_BIN / "llvm-nm"
    if not path.exists():
        sys.exit(f"{path} fehlt -- llvm-mos-SDK installieren (siehe README) "
                 f"oder LLVM_MOS_BIN setzen")
    out = subprocess.run([str(path), "--radix=d", elf], check=True,
                         capture_output=True, text=True).stdout
    found = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[0].isdigit():
            found[parts[2]] = int(parts[0])
    return found


def port_free(timeout=15.0):
    """Wartet, bis auf PORT nichts mehr lauscht -- ein x64sc aus einem
    vorherigen Lauf wuerde den Port festhalten und der neue bekaeme still
    keinen Monitor."""
    end = time.time() + timeout
    while time.time() < end:
        try:
            socket.create_connection(("127.0.0.1", PORT), timeout=1).close()
        except OSError:
            return
        time.sleep(0.5)
    sys.exit(f"auf Port {PORT} lauscht schon etwas -- laeuft noch ein x64sc?")


class Monitor:
    """Eine Verbindung zum VICE-Monitor, die sich nach `resume()` schliesst."""

    def __init__(self, emu=None, timeout=20.0):
        end = time.time() + timeout
        while True:
            try:
                self.sock = socket.create_connection(("127.0.0.1", PORT), timeout=5)
                break
            except OSError:
                if emu is not None and emu.poll() is not None:
                    raise SystemExit(f"x64sc hat sich beendet (Code {emu.returncode})")
                if time.time() >= end:
                    raise SystemExit("keine Verbindung zum VICE-Monitor")
                time.sleep(0.1)
        self.sock.settimeout(1.0)

    def command(self, text):
        self.sock.sendall((text + "\n").encode())
        out = b""
        while True:
            try:
                chunk = self.sock.recv(65536)
            except (socket.timeout, OSError):
                break
            if not chunk:
                break
            out += chunk
        return out.decode("latin-1")

    def read(self, addr, count):
        """`count` Bytes ab `addr`, aus der Ausgabe von `m`.

        VICE antwortet je Zeile mit `>C:<adresse>  <bis zu 16 Bytes>  <text>`;
        genommen werden nur Zeilen, die die Reihe ab `addr` fortsetzen. Hinter
        den Bytes steht der Text derselben Stelle -- was davon wie eine Zahl
        aussieht, faellt beim Abschneiden auf `count` weg."""
        text = self.command(f"m {addr:04x} {addr + count - 1:04x}")
        data = []
        for line in text.splitlines():
            hit = re.search(r">\s*C:([0-9a-f]{4})((?: +[0-9a-f]{2}){1,16})", line)
            if hit and int(hit.group(1), 16) == addr + len(data):
                data += [int(b, 16) for b in hit.group(2).split()]
                if len(data) >= count:
                    return data[:count]
        raise SystemExit(f"Monitor-Antwort nicht verstanden:\n{text}")

    def resume(self):
        try:
            self.sock.sendall(b"x\n")
        except OSError:
            pass
        self.sock.close()

    def quit(self):
        try:
            self.sock.sendall(b"quit\n")
        except OSError:
            pass
        self.sock.close()


def little(data):
    return sum(b << (8 * i) for i, b in enumerate(data))


def main():
    prg = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "build/bench.prg")
    elf = prg.with_suffix(".prg.elf") if prg.suffix == ".prg" else prg
    if not elf.exists():
        sys.exit(f"{elf} fehlt -- erst `make build/bench.prg`")
    if not os.environ.get("DISPLAY"):
        sys.exit("DISPLAY fehlt -- x64sc ist mit GTK gebaut und braucht eine Anzeige")

    syms = symbols(str(elf))
    try:
        ready, cycles, overhead, rounds = (
            syms["bench_ready"], syms["bench_cycles"],
            syms["bench_overhead"], syms["bench_rounds"])
    except KeyError as missing:
        sys.exit(f"{missing} fehlt im ELF -- ist {prg} aus tools/bench_render.c gebaut?")

    port_free()
    # BENCH_LOG=datei haelt fest, was x64sc sagt -- sonst redet der Emulator
    # ueber den Bericht.
    log = open(os.environ["BENCH_LOG"], "w") if os.environ.get("BENCH_LOG") \
        else subprocess.DEVNULL
    emu = subprocess.Popen(
        ["x64sc", "-default", "+sound", "-warp", "-autostart-warp",
         "-limitcycles", str(LIMIT_CYCLES), "-remotemonitor",
         "-remotemonitoraddress", f"ip4://127.0.0.1:{PORT}", "-autostart", str(prg)],
        stdout=log, stderr=subprocess.STDOUT)
    try:
        time.sleep(WAIT_FIRST)
        for attempt in range(TRIES):
            mon = Monitor(emu)
            if mon.read(ready, 1)[0] == READY:
                total = little(mon.read(cycles, 4))
                empty = little(mon.read(overhead, 4))
                runs = mon.read(rounds, 1)[0]
                mon.quit()
                break
            mon.resume()
            time.sleep(0.5)
        else:
            sys.exit("bench.prg ist nicht fertig geworden -- laeuft es in VICE?")
    finally:
        try:
            emu.wait(timeout=5)
        except subprocess.TimeoutExpired:
            emu.kill()

    if not runs:
        sys.exit("bench_rounds ist 0 -- Messung unbrauchbar")
    per = (total - empty) / runs
    print(f"Bildaufbau: {prg} in x64sc")
    print()
    print(f"  {'Bilder gezeichnet':<28}{runs:>9}")
    print(f"  {'Takte zusammen':<28}{total:>9}")
    print(f"  {'davon Messfenster':<28}{empty:>9}")
    print(f"  {'Takte je Bild':<28}{round(per):>9}")
    print(f"  {'das sind bei 1 MHz':<28}{per / 1000:>8.1f} ms")


if __name__ == "__main__":
    main()
