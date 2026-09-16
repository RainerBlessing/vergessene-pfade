# Third-party notices

SDL3 (zlib license) is used as the platform library. Linux development builds link
the system SDL3 package. Distributed builds (Stage 6) will statically link SDL 3.2.28,
unpacked unmodified from the official release archive into `vendor/`:
https://github.com/libsdl-org/SDL/releases/download/release-3.2.28/SDL3-3.2.28.tar.gz
Those packages must ship `vendor/SDL3-3.2.28/LICENSE.txt`. SDL also provides the bundled
bitmap font used for text.

The code base derives from the author's Emberpost proof of concept (see `docs/emberpost/`).
Maps, palette, atlas generator, sprites, names and dialogue are original.

Retro-RPG structure reference: https://nippon-museum.de (fan archive of the C64 game NIPPON).
Thematic inspiration: H. Yoda / S. Blind, "Acht Millionen Wege zum Glück" (dtv).
No images, text or game content from either source are distributed in this project.
