# Milestone verification log

1. **Window and renderer**: configured and compiled vendored SDL3; disabled precompiled headers after a MinGW mapping error. Launched the native 1280x800 window and inspected a captured 320x200 integer-scaled tile grid with a visible player before proceeding.
2. **Exploration**: extracted the SDL-free game model and map loader. Built, ran collision/movement/camera checks, and launched the game.
3. **Town**: added external town data, two transitions, and four data-defined NPCs. Built and tested entry, exit, arrival position, and NPC collision; launched the game.
4. **Dialogue**: added facing-based interactions and paged content. Fixed a generated string-literal compile error, then built and tested opening, advancing, and closing dialogue; launched the game.
5. **RPG systems**: added statistics, inventory, herbs, automatic sword equipment, and purchases. Built and tested quantity limits, insufficient funds, healing, and menus; launched the game.
6. **Quest**: added explicit quest states and conditional dialogue. Built and tested the objective/reward transitions and rejection of duplicate completion; launched the game.
7. **Combat**: added the encounter, attack/heal turns, damage, victory, and town recovery. Built and tested both outcomes and quest-item delivery; launched the game.
8. **Vertical slice**: passed a full start-to-completion journey using actual movement and interaction actions, with no state editing. Only then replaced geometric art with the original atlas, refined map details, and added journal/debug UI. Native screenshot review caught SDL bitmap-font line handling; fixed it and inspected the resulting screens.

Every milestone was rebuilt and summarized before the next. Verification used Windows x64 with MinGW-w64 GCC, CMake, and SDL3 3.2.28. The tests intentionally remain active in Release builds. A subsequent clean-source build, packaged-game launch, and final test results are recorded in `validation.txt`.
