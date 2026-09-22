#!/bin/bash
# Build the C64 POC and start it in VICE.
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
make -C "$SCRIPT_DIR" build
exec x64sc -default -autostart "$SCRIPT_DIR/build/vergessene-pfade.prg" "$@"
