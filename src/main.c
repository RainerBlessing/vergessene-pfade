#include "journey.h"
#include "renderer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <string.h>
static Action key(SDL_Keycode k) {
  switch (k) {
  case SDLK_UP:
  case SDLK_W:
    return ACT_UP;
  case SDLK_DOWN:
  case SDLK_S:
    return ACT_DOWN;
  case SDLK_LEFT:
  case SDLK_A:
    return ACT_LEFT;
  case SDLK_RIGHT:
  case SDLK_D:
    return ACT_RIGHT;
  case SDLK_RETURN:
  case SDLK_SPACE:
    return ACT_CONFIRM;
  case SDLK_ESCAPE:
    return ACT_CANCEL;
  case SDLK_I:
    return ACT_INVENTORY;
  case SDLK_F1:
    return ACT_DEBUG;
  case SDLK_F2:
    return ACT_COLLISION;
  default:
    return ACT_NONE;
  }
}
static Action held(void) {
  const bool *k = SDL_GetKeyboardState(NULL);
  if (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W])
    return ACT_UP;
  if (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S])
    return ACT_DOWN;
  if (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A])
    return ACT_LEFT;
  if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D])
    return ACT_RIGHT;
  return ACT_NONE;
}
typedef struct {
  Renderer *renderer;
  bool ok;
} Capture;
static void capture(const Game *g, const char *label, void *context) {
  Capture *c = context;
  render_game(c->renderer, g, 60);
  SDL_Surface *s = SDL_RenderReadPixels(c->renderer->sdl, NULL);
  char path[128];
  snprintf(path, sizeof path, "%s.bmp", label);
  if (!s || !SDL_SaveBMP(s, path)) {
    fprintf(stderr, "Bildaufnahme fehlgeschlagen: %s\n", SDL_GetError());
    c->ok = false;
  }
  SDL_DestroySurface(s);
  SDL_RenderPresent(c->renderer->sdl);
  SDL_PumpEvents();
}
int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "%s\n", SDL_GetError());
    return 1;
  }
  SDL_Window *w = NULL;
  SDL_Renderer *sdl = NULL;
  Renderer r = {0};
  int result = 1;
  if (!SDL_CreateWindowAndRenderer("Die vergessenen Pfade | POC Walddorf", 1280, 800,
                                   SDL_WINDOW_RESIZABLE, &w, &sdl))
    goto cleanup;
  SDL_SetWindowMinimumSize(w, 320, 200);
  if (!SDL_SetRenderLogicalPresentation(sdl, 320, 200,
                                        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE))
    goto cleanup;
  char assets[1024];
  snprintf(assets, sizeof assets, "%sassets", SDL_GetBasePath());
  Game g;
  if (!game_init(&g, assets) || !renderer_init(&r, sdl, assets)) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR, "Die vergessenen Pfade",
        "Spieldaten fehlen. Der Ordner assets muss neben dem Programm liegen. Bitte das "
        "Spiel vollstaendig entpacken.",
        w);
    goto cleanup;
  }
  if (argc > 1 && strcmp(argv[1], "--verify") == 0) {
    Capture c = {&r, true};
    result = journey(&g, capture, &c) && c.ok ? 0 : 1;
    goto cleanup;
  }
  bool run = true, smoke = argc > 1 && strcmp(argv[1], "--smoke") == 0;
  int frames = 0, fps = 60, fps_frames = 0;
  Uint64 last_move = 0, fps_time = SDL_GetTicks();
  result = 0;
  while (run) {
    Uint64 frame_start = SDL_GetTicks();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        run = false;
      if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
        if (g.state == GAME_PAUSED && e.key.key == SDLK_Q)
          run = false;
        else if (g.state == GAME_PAUSED && e.key.key == SDLK_R) {
          if (!game_init(&g, assets)) {
            result = 1;
            run = false;
          }
        } else {
          game_action(&g, key(e.key.key));
          last_move = frame_start;
        }
      }
    }
    if (g.state == GAME_EXPLORATION && (SDL_GetWindowFlags(w) & SDL_WINDOW_INPUT_FOCUS) &&
        frame_start - last_move >= 140) {
      Action a = held();
      if (a != ACT_NONE) {
        game_action(&g, a);
        last_move = frame_start;
      }
    }
    if (frame_start - fps_time >= 1000) {
      fps = (int)(fps_frames * 1000 / (frame_start - fps_time));
      fps_frames = 0;
      fps_time = frame_start;
    }
    fps_frames++;
    render_game(&r, &g, fps);
    if (smoke && ++frames == 10) {
      Capture c = {&r, true};
      capture(&g, "smoke", &c);
      result = c.ok ? 0 : 1;
      run = false;
    } else
      SDL_RenderPresent(sdl);
    Uint64 elapsed = SDL_GetTicks() - frame_start;
    if (elapsed < 16)
      SDL_Delay((Uint32)(16 - elapsed));
  }
cleanup:
  if (result)
    fprintf(stderr, "Die vergessenen Pfade fehlgeschlagen: %s\n", SDL_GetError());
  renderer_destroy(&r);
  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(w);
  SDL_Quit();
  return result;
}
