#ifndef RENDERER_H
#define RENDERER_H
#include "game.h"
#include <SDL3/SDL.h>
typedef struct {
  SDL_Renderer *sdl;
  SDL_Texture *atlas;
  bool paper;
  const char *audio; /* the sound setting, shown in the notebook */
} Renderer;
bool renderer_init(Renderer *r, SDL_Renderer *sdl, const char *assets);
void renderer_destroy(Renderer *r);
void render_game(Renderer *r, const Game *g, int fps);
#endif
