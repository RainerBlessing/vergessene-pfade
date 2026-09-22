LLVM_MOS_CLANG := $(HOME)/.local/share/llvm-mos-sdk/bin/clang
PRG := build/vergessene-pfade.prg
SOURCES := src/main.c src/render.c src/screen.c src/game.c src/content.c src/world.c
CORE := src/game.c src/content.c src/world.c
VIEW := src/render.c src/screen.c
HOST_CC := cc -std=c11 -Wall -Wextra -Werror

.PHONY: build test maps run clean

build: $(PRG)

$(PRG): $(SOURCES) src/maps.h
	mkdir -p build
	$(LLVM_MOS_CLANG) --target=mos-c64 -Os -o $(PRG) $(SOURCES)

# Neither the rules nor the screen layout need a C64, so both are played
# through on the host; tests/c64.h stands in for the SDK header.
test: build/test_game build/test_screen
	./build/test_game
	./build/test_screen

build/test_game: tests/test_game.c $(CORE) src/maps.h
	mkdir -p build
	$(HOST_CC) -o $@ tests/test_game.c $(CORE)

build/test_screen: tests/test_screen.c tests/c64.h $(CORE) $(VIEW) src/maps.h
	mkdir -p build
	$(HOST_CC) -Itests -o $@ tests/test_screen.c $(VIEW) $(CORE)

maps:
	python3 tools/genmaps.py

run: $(PRG)
	./run-vice.sh

clean:
	rm -rf build
