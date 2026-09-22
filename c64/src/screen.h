#ifndef SCREEN_H
#define SCREEN_H
#include <stdint.h>

#define SCREEN_COLS 40
#define SCREEN_ROWS 25

/* Where the characters go. The host tests point these at plain arrays so the
 * layout can be checked without a C64. */
#ifndef SCREEN_RAM
#define SCREEN_RAM ((volatile uint8_t *)0x0400)
#endif

/* Lowercase character set, black screen, cleared. */
void screen_init(void);
void screen_clear(void);
void screen_put(uint8_t x, uint8_t y, uint8_t code, uint8_t color);
/* Fills the row with spaces first, then writes the text. */
void screen_row(uint8_t y, uint8_t color);
/* One line of ASCII, stopping at the end of the line or at '\n'.
 * Returns what is left of the text after the line break, or 0. */
const char *screen_text(uint8_t x, uint8_t y, const char *text, uint8_t color);
/* Several lines, one per '\n'; returns the row after the last one. */
uint8_t screen_lines(uint8_t x, uint8_t y, const char *text, uint8_t color);
#endif
