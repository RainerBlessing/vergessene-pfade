#include <c64.h> /* the host tests provide a stand-in that redirects
                  * SCREEN_RAM and COLOR_RAM into plain arrays */
#include "screen.h"

/* PETSCII screen codes of the lowercase character set: a-z are 1-26, A-Z stay
 * 65-90, and everything from space to '?' keeps its ASCII value. */
static uint8_t screen_code(char c) {
  uint8_t u = (uint8_t)c;
  if (u >= 'a' && u <= 'z')
    return (uint8_t)(u - 96);
  if (u >= 'A' && u <= 'Z')
    return u;
  if (u >= 32 && u < 64)
    return u;
  return 32;
}

void screen_init(void) {
  VIC.addr = 0x17; /* screen at $0400, lowercase character set */
  VIC.bordercolor = COLOR_BLACK;
  VIC.bgcolor0 = COLOR_BLACK;
  screen_clear();
}

void screen_clear(void) {
  for (uint16_t i = 0; i < SCREEN_COLS * SCREEN_ROWS; i++) {
    SCREEN_RAM[i] = 32;
    COLOR_RAM[i] = COLOR_WHITE;
  }
}

/* One multiplication per row instead of one per character cell. */
static const uint16_t row_offset[SCREEN_ROWS] = {
    0,   40,  80,  120, 160, 200, 240, 280, 320, 360, 400, 440, 480,
    520, 560, 600, 640, 680, 720, 760, 800, 840, 880, 920, 960};

void screen_put(uint8_t x, uint8_t y, uint8_t code, uint8_t color) {
  uint16_t at = row_offset[y] + x;
  SCREEN_RAM[at] = code;
  COLOR_RAM[at] = color;
}

void screen_row(uint8_t y, uint8_t color) {
  for (uint8_t x = 0; x < SCREEN_COLS; x++)
    screen_put(x, y, 32, color);
}

const char *screen_text(uint8_t x, uint8_t y, const char *text, uint8_t color) {
  while (*text && *text != '\n' && x < SCREEN_COLS)
    screen_put(x++, y, screen_code(*text++), color);
  while (*text && *text != '\n') /* anything past the edge is dropped */
    text++;
  return *text == '\n' ? text + 1 : 0;
}

uint8_t screen_lines(uint8_t x, uint8_t y, const char *text, uint8_t color) {
  while (text && y < SCREEN_ROWS) {
    screen_row(y, color);
    text = screen_text(x, y++, text, color);
  }
  return y;
}
