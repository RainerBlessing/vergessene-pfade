# Emberpost: The Lantern Path

A complete, original C11 / SDL3 RPG proof of concept. Restore a village beacon by recovering its amber mirror from a thorn spirit. Explore a 48x40 overworld and 32x32 town, speak to four villagers, buy supplies, fight, and return for your reward.

![Aoi's request](docs/screenshot.png)

## Japanese-inspired edition

The current edition uses original torii gates, sakura, bamboo, flooded rice paddies, wooden bridges, shoji walls, tiled roofs, tatami floors, kimono-style robes and paper dialogue panels. Koharu's shrine keeper Aoi asks you to recover an amber mirror from a fictional thorn spirit. The villagers are human; the spirit is not assigned the identity of a specific kami.

Visual reference: [Nippon Museum gallery](https://nippon-museum.de/gallery). Only the broad retro presentation and Japanese atmosphere informed this edition; its artwork, maps, names and dialogue are original.

## Play the Windows build

Extract **Emberpost-Japan.zip** and run `emberpost.exe`. Keep the accompanying `assets` directory next to it. No SDL installation is required. This is a native Windows x64 executable, not a browser game.

## Build from source

Requirements: CMake 3.20+, a C11 compiler, and your compiler's build tool. SDL **3.2.28** source is vendored with its license under `vendor/`; no network access or additional package manager is needed. Python is only needed if you choose to regenerate the art, not to build or play.

### Windows: Visual Studio developer terminal

Install Visual Studio's Desktop development with C++ workload (it includes a C compiler and Windows SDK), then run:

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
build\Release\emberpost.exe
```

### Windows: MinGW-w64 GCC and GNU Make

With `gcc` and `make` on PATH (the toolchain used for verification):

```sh
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 8
ctest --test-dir build --output-on-failure
build\emberpost.exe
```

Use a separate build directory if changing compiler or generator. Precompiled headers are disabled to avoid a MinGW address-mapping failure encountered during development.

### Linux

Install a C compiler, CMake, Make, and native window-system development packages. For example on Debian/Ubuntu:

```sh
sudo apt install build-essential cmake pkg-config libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev libxss-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/emberpost
```

SDL detects available display backends; the packages above provide an X11 build. See SDL's vendored `docs/README-linux.md` for Wayland and other platform dependencies. Linux and MSVC instructions are provided, but these platforms were not executed in this Windows environment.

Assets are copied beside the executable by CMake on every build. Runtime paths use the executable's directory, so launching from a different working directory works. To stage a portable game folder:

```sh
cmake --install build --config Release --prefix dist
```

## Controls

| Key | Action |
| --- | --- |
| Arrow keys / WASD | Move one tile; hold to continue |
| Space / Enter | Talk to the NPC you face; advance dialogue; confirm a menu choice |
| Up / Down | Select inventory, shop, or combat action |
| I | Open inventory; close it when already open |
| Escape | Close dialogue/menu; open or close the pause journal |
| F1 | Toggle map, position, FPS, state, quest, and NPC debug display |
| F2 | Mark blocked tiles |
| R, while paused | Start a new journey |
| Q, while paused | Quit |

An attempted step toward an NPC changes your facing, without moving into them. Then press Enter to talk. Shop purchases and combat actions require a fresh key press; keyboard auto-repeat cannot spend your gold or take extra combat turns.

## The vertical slice

1. Follow the starting trail north and step through the town gate.
2. Find Aoi inside the north-west house. Her request starts the single quest.
3. Speak to Nao near the south gate for directions; Hana provides village flavour.
4. Enter Ren's north-east shop. Finish his dialogue to see his goods. Tea portions cost 6 gold and heal up to 8 HP. A 16-gold travel blade adds 2 attack and equips automatically.
5. Leave south. Follow the trail south to its junction, east across the river, then north to the forest shrine.
6. Step onto the beast to enter combat. Attack or use a tea; a successful action allows the beast to retaliate. Failed item use costs no turn. Defeat returns you to town with full health and a replacement tea.
7. Victory grants the mirror. Return to Aoi for 25 gold, full healing, and the ending dialogue. The reward cannot be claimed again.

The intended first visit is approximately 5-15 minutes, including exploration and reading; this is not a measured playtime study. A player who knows the route can finish faster. The pause journal always shows the current objective. There is no save file; closing or restarting begins a new journey.

## Architecture and content

- `src/game.*`: state machine, grid movement, interactions, and transitions.
- `src/world.*`: bounded map parser and central tile properties.
- `src/content.*`: read-only NPC positions, roles, transitions, and paged dialogue.
- `src/inventory.*`, `quest.*`, `combat.*`: SDL-free RPG rules and compact state.
- `src/main.c`: SDL lifecycle, input, timing, asset-path resolution, and diagnostic capture.
- `src/renderer.*`: SDL 2D rendering, camera projection, bitmap text, and overlays.
- `assets/maps/`: external plain-text maps. `assets/tiles/`: original BMP atlas and optional regeneration script.
- `tests/`: headless unit checks and a full journey driven exclusively through game actions.

The 320x200 logical display uses 16x16 tiles, integer scaling, and nearest-neighbour textures. Art uses a small original palette. All game art, names, story, maps, and dialogue were created for this POC; no Nippon assets or text are included. Text uses SDL's bundled bitmap font.

See [architecture](docs/architecture.md), [milestone log](docs/milestones.md), and [third-party notices](THIRD_PARTY.md).

## Verification

```sh
ctest --test-dir build --output-on-failure
```

Two tests cover inventory limits, buying and healing, map collision, movement and camera, town transitions, dialogue, quest transitions and duplicate rewards, damage bounds, victory/defeat, and a complete traversable journey. The journey uses pathfinding only to select movement inputs: it never teleports the player or modifies quest/player state.

For an SDL window and screenshots of the actual journey screens, run `emberpost --verify` from a writable scratch directory. It writes numbered BMP captures and exits successfully only if the journey and captures pass. `--smoke` captures ten frames after startup and exits. These diagnostics do not require physical keyboard automation.

Core-only builds do not configure or link SDL:

```sh
cmake -S . -B build-core -DEMBERPOST_BUILD_GAME=OFF
cmake --build build-core
ctest --test-dir build-core --output-on-failure
```

## Known limitations

One quest, one non-respawning encounter, stationary NPCs, no selling, audio, save/load, gamepad support, animation, or progression beyond the ending. Cottage interiors share the town map. Content tables require recompilation; maps and the atlas are external. Combat uses a fixed seeded sequence for reproducibility. Text is English ASCII with authored line breaks. This is an architectural starting point for a possible C64U experiment, not a C64 implementation or a general-purpose RPG engine.
