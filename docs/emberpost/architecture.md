# Architecture

## State and ownership

`Game` owns two maps, player statistics/inventory, position and facing, quest state, combat state, and menu/dialogue cursors. The frontend owns the SDL window, renderer, and one texture atlas. Core code includes no SDL headers and performs no graphics, keyboard, or timing calls. `game_action` accepts an enum and dispatches by state: exploration, dialogue, inventory, shop, combat, or paused. Movement and combat are action-driven; idle frames never advance simulation.

## Maps, tiles, and camera

Each external map begins with `width height`, followed by exactly that many ASCII rows. Dimensions are bounded at 64x64. The parser rejects unknown symbols, truncated rows, and invalid dimensions before replacing a map. Maps use flat arrays of one-byte symbols. A single immutable tile table supplies passability and transition metadata; collision does not depend on scattered numeric tile IDs. NPC occupancy also blocks movement. A small transition table connects gates to safe arrival tiles, avoiding immediate transition loops.

`game_camera` clamps a tile-aligned 20x10 view around the player. The renderer translates map positions to the viewport beneath the 16-pixel header; coordinates in the game model remain integers. Bitmap atlas IDs are renderer details, with terrain symbols mapped in one place. SDL scales the 320x200 logical output by an integer factor and uses nearest-neighbour sampling.

## Characters and content

Four `Npc` records hold map, position, sprite ID, role, and display name. There are no individual NPC update functions. Dialogue consists of small immutable arrays of pages. The active dialogue ID and page cursor are stored independently of the NPC ID, allowing the keeper to select different dialogue for each quest state. Pages have authored line breaks and are tested for panel width/height. The SDL font requires the renderer to split these lines explicitly.

## RPG rules

Inventory is a bounded quantity array indexed by item ID, with checked add/remove operations. Purchases update gold only after the item can be added. The travel blade is unique and automatically raises attack; tea portions clamp healing to maximum HP.

The single quest uses `NOT_STARTED -> ACTIVE -> OBJECTIVE_FOUND -> COMPLETED`. Each transition rejects invalid or repeated calls. Victory creates the mirror; completion consumes it before granting gold and healing. Accepting the quest and handing in the mirror happen on initiating the corresponding conversation, so cancelling text does not partially undo the transaction.

Combat is turn-based. Damage is `max(1, attack - defense + modifier)`, with a seeded unsigned LCG producing modifiers from -1 to +1. A failed tea use does not consume a turn. Victory/defeat are explicit terminal combat flags; Enter acknowledges the result. Defeat preserves the active quest and sends the player to town. There is no combat timer or frame-rate dependency.

## Platform boundary and C64U prospects

`main.c` is the small platform layer: SDL startup/shutdown, keyboard translation, held-key cadence, timing, executable-relative paths, and capture diagnostics. `renderer.c` contains all drawing and texture operations. `world.c` uses standard C file I/O only during initialization; a C64U loader could replace this with compiled binary map data. No per-frame game allocations occur. Formatted HUD strings use stack buffers. SDL manages its own backend resources.

The state machine, inventory/quest rules, combat arithmetic, tile tables, and map/NPC data are plausible shared starting points. A port would replace input/rendering/timing/file loading, convert pointer-based text tables to IDs or offsets if needed, narrow integer fields, and pack map data. The current two 64x64-capacity map arrays and C strings deliberately do not enforce actual C64 memory limits. SDL's font and atlas are frontend assets, not required by the core model.

## Tests

`test_game` checks rules and boundary cases without SDL. `test_journey` navigates the real maps and completes the quest using only `game_action`; it catches disconnected content and transition mistakes. The same journey has an optional observer used by `--verify` to capture native SDL screens. That observer cannot mutate the game. The project is a small game; no ECS, scripting engine, or generic platform framework is introduced.
