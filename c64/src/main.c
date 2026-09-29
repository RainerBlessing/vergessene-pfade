/* Die vergessenen Pfade -- C64 POC.
 *
 * Text mode, 40x25: the map scrolls in the upper part, texts stand below it.
 * The Kernal's own interrupt keeps running, so the keyboard buffer works and
 * cbm_k_getin() is all the input this needs. Sound plays in this loop too:
 * every effect is short (sound.h), so no interrupt of our own is needed. */
#include "game.h"
#include "render.h"
#include "screen.h"
#include "sound.h"
#include <cbm.h>

#define KEY_RETURN 13
#define KEY_STOP 3
#define KEY_CURSOR_DOWN 17
#define KEY_CURSOR_RIGHT 29
#define KEY_CURSOR_UP 145
#define KEY_CURSOR_LEFT 157

static Action action_for(uint8_t key) {
  if (key >= 'a' && key <= 'z') /* PETSCII: unshifted letters are 65-90 */
    key = (uint8_t)(key - 32);
  switch (key) {
  case 'W':
  case KEY_CURSOR_UP:
    return ACT_UP;
  case 'S':
  case KEY_CURSOR_DOWN:
    return ACT_DOWN;
  case 'A':
  case KEY_CURSOR_LEFT:
    return ACT_LEFT;
  case 'D':
  case KEY_CURSOR_RIGHT:
    return ACT_RIGHT;
  case KEY_RETURN:
  case ' ':
    return ACT_CONFIRM;
  case 'I':
    return ACT_INVENTORY;
  case 'N':
  case KEY_STOP:
    return ACT_CANCEL;
  default:
    return ACT_NONE;
  }
}

int main(void) {
  static Game game;
  static RenderCache cache = {0};
  screen_init();
  sound_init();
  game_init(&game);
  render(&game, &cache);
  for (;;) {
    uint8_t key = cbm_k_getin();
    if (!key)
      continue;
    Action a = action_for(key);
    if (a == ACT_NONE)
      continue;
    game_action(&game, a);
    render(&game, &cache);
    sound_play((SfxId)game.sfx);
  }
}
