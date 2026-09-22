/* Stand-in for the SDK's <c64.h> so screen.c and render.c can be compiled and
 * checked on the host. Only what those two files use. */
#ifndef HOST_C64_H
#define HOST_C64_H
#include <stdint.h>

#define COLOR_BLACK 0
#define COLOR_WHITE 1
#define COLOR_RED 2
#define COLOR_CYAN 3
#define COLOR_PURPLE 4
#define COLOR_GREEN 5
#define COLOR_BLUE 6
#define COLOR_YELLOW 7
#define COLOR_ORANGE 8
#define COLOR_BROWN 9
#define COLOR_LIGHTRED 10
#define COLOR_GRAY1 11
#define COLOR_GRAY2 12
#define COLOR_LIGHTGREEN 13
#define COLOR_LIGHTBLUE 14
#define COLOR_GRAY3 15

extern uint8_t test_screen[1000], test_color[1000];
#undef SCREEN_RAM
#define SCREEN_RAM test_screen
#define COLOR_RAM test_color

struct host_vic {
  uint8_t addr, bordercolor, bgcolor0;
};
extern struct host_vic VIC;
#endif
