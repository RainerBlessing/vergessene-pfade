#include "audio.h"
#include "journey.h"
#include "options.h"
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
/* Session log: one tab-separated line per event (see IMPLEMENTATION_PLAN.md). */
static void drain_events(Game *g, FILE *log, Uint64 ms, Audio *audio) {
  GameEvent events[EVENT_LIMIT];
  int n = game_take_events(g, events, EVENT_LIMIT);
  if (audio)
    audio_events(audio, events, n);
  if (!log)
    return;
  for (int i = 0; i < n; i++) {
    const GameEvent *e = &events[i];
    fprintf(log, "%llu\t%d\t%d,%d\t", (unsigned long long)ms, e->map, e->x, e->y);
    switch (e->type) {
    case EV_OBSERVE:
      fprintf(log, "observe\t%s\n", obs_names[e->a]);
      break;
    case EV_EXAMINE:
      fprintf(log, "examine\ttile=%c dialogue=%d\n", e->a ? e->a : '-', e->b);
      break;
    case EV_EXAMINE_NOTHING:
      fprintf(log, "examine_nothing\ttile=%c repeat=%d\n", e->a, e->b);
      break;
    case EV_NPC_TALK:
      fprintf(log, "npc_talk\t%s dialogue=%d\n", npcs[e->a].key, e->b);
      break;
    case EV_NOTEBOOK_OPEN:
      fprintf(log, "notebook_open\tentries=%d\n", e->a);
      break;
    case EV_ENCOUNTER:
      fprintf(log, "encounter\tmood=%s\n", mood_names[e->a]);
      break;
    case EV_STONE_PUSH:
      fprintf(log, "stone_push\tstone=%d,%d\n", e->a, e->b);
      break;
    case EV_MEND:
      fprintf(log, "mend\tplaced=%d fits=%d\n", e->a, e->b);
      break;
    case EV_STAKE:
      fprintf(log, "stake\tset=%d\n", e->a);
      break;
    case EV_PHASE:
      fprintf(log, "phase\t%s\n", phase_names[e->a]);
      break;
    case EV_OUTCOME:
      fprintf(log, "outcome\t%s\n", outcome_names[e->a]);
      break;
    case EV_ENCOUNTER_ACTION:
      fprintf(log, "encounter_action\t%s mood=%s\n", encounter_action_names[e->a],
              mood_names[e->b]);
      break;
    case EV_ITEM_USE:
      fprintf(log, "action_attempt\tuse=%s ok=%d dialogue=%d\n", items[e->a].name,
              e->b != D_NONE, e->b);
      break;
    }
  }
  if (g->events_dropped) {
    fprintf(log, "%llu\t%d\t%d,%d\tevents_dropped\tcount=%d\n", (unsigned long long)ms,
            g->map, g->x, g->y, g->events_dropped);
    g->events_dropped = 0;
  }
  fflush(log);
}
static int usage(void) {
  fprintf(stderr, "Aufruf: vergessene_pfade [--smoke | --verify] [--log DATEI]"
                  " [--scale 2..5] [--fullscreen]\n");
  return 2;
}
typedef struct {
  Renderer *renderer;
  FILE *log;
  bool ok;
} Capture;
static void capture(Game *g, const char *label, void *context) {
  Capture *c = context;
  drain_events(g, c->log, SDL_GetTicks(), NULL);
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
/* One run per journey; each starts from a fresh game. */
static bool run_verify(Renderer *r, FILE *log, const char *assets, Game *g, char *reason,
                       size_t reason_size) {
  Capture c = {r, log, true};
  static const struct {
    const char *name;
    bool (*run)(Game *, JourneyObserver, void *);
  } runs[] = {{"exploration", journey},
              {"fight", journey_fight},
              {"boundary", journey_boundary},
              {"mend", journey_mend}};
  for (size_t i = 0; i < sizeof runs / sizeof runs[0]; i++) {
    if (log)
      fprintf(log, "# run\t%s\n", runs[i].name);
    bool loaded = game_init(g, assets);
    bool passed = loaded && runs[i].run(g, capture, &c);
    drain_events(g, log, SDL_GetTicks(), NULL); /* before game_init clears them */
    if (!loaded)
      snprintf(reason, reason_size, "Spieldaten fuer Abnahme \"%s\" nicht ladbar",
               runs[i].name);
    else if (!c.ok)
      snprintf(reason, reason_size, "Bildaufnahme in Abnahme \"%s\" fehlgeschlagen",
               runs[i].name);
    else if (!passed)
      snprintf(reason, reason_size, "Abnahme \"%s\" ist fehlgeschlagen", runs[i].name);
    else
      continue;
    return false;
  }
  return true;
}
/* The interactive game: one event per frame, the picture paced to 60 fps. */
static bool run_loop(Renderer *r, SDL_Renderer *sdl, SDL_Window *w, const char *assets,
                     Game *g, Audio *audio, FILE *log, const Options *o,
                     const char **reason) {
  audio_open(audio);
  bool run = true;
  int scale = o->scale, frames = 0, fps = 60, fps_frames = 0;
  bool fullscreen = o->fullscreen;
  Uint64 last_move = 0, fps_time = SDL_GetTicks();
  /* One confirmation sounds once: the specific sounds come from the events. */
  while (run) {
    Uint64 frame_start = SDL_GetTicks();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        run = false;
      if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
        if (e.key.key == SDLK_F3) {
          audio_cycle(audio);
          audio_play(audio, SFX_CLICK);
        } else if (e.key.key == SDLK_F11) {
          fullscreen = !fullscreen;
          SDL_SetWindowFullscreen(w, fullscreen);
        } else if (e.key.key == SDLK_F4) { /* one step larger, then back to 2x */
          scale = scale >= SCALE_MAX ? SCALE_MIN : scale + 1;
          fullscreen = false;
          SDL_SetWindowFullscreen(w, false);
          SDL_SetWindowSize(w, 320 * scale, 200 * scale);
        } else if (g->state == GAME_NOTEBOOK && e.key.key == SDLK_Q)
          run = false;
        else if (g->state == GAME_NOTEBOOK && e.key.key == SDLK_R) {
          if (!game_init(g, assets)) {
            *reason = "Spieldaten konnten beim Neustart nicht geladen werden";
            return false;
          }
        } else {
          unsigned before = g->steps;
          bool answering =
              g->state != GAME_EXPLORATION &&
              (key(e.key.key) == ACT_CONFIRM || key(e.key.key) == ACT_CANCEL);
          Action a = key(e.key.key);
          game_action(g, a);
          if (answering)
            audio_play(audio, SFX_CLICK);
          drain_events(g, log, SDL_GetTicks(), audio);
          if (g->steps != before)
            audio_footstep(audio);
          last_move = frame_start;
        }
      }
    }
    if (g->state == GAME_EXPLORATION &&
        (SDL_GetWindowFlags(w) & SDL_WINDOW_INPUT_FOCUS) &&
        frame_start - last_move >= 140) {
      Action a = held();
      if (a != ACT_NONE) {
        unsigned before = g->steps;
        game_action(g, a);
        drain_events(g, log, SDL_GetTicks(), audio);
        if (g->steps != before)
          audio_footstep(audio);
        last_move = frame_start;
      }
    }
    if (frame_start - fps_time >= 1000) {
      fps = (int)(fps_frames * 1000 / (frame_start - fps_time));
      fps_frames = 0;
      fps_time = frame_start;
    }
    fps_frames++;
    audio_update(audio);
    r->audio = audio_label(audio);
    char display[48];
    snprintf(display, sizeof display, "F4 FENSTER %dx  F11 %s", scale,
             fullscreen ? "FENSTERMODUS" : "VOLLBILD");
    r->display = display;
    render_game(r, g, fps);
    if (o->smoke && ++frames == 10) {
      Capture c = {r, log, true};
      capture(g, "smoke", &c);
      if (!c.ok)
        *reason = "Bildaufnahme fehlgeschlagen";
      return c.ok;
    } else
      SDL_RenderPresent(sdl);
    Uint64 elapsed = SDL_GetTicks() - frame_start;
    if (elapsed < 16)
      SDL_Delay((Uint32)(16 - elapsed));
  }
  return true;
}
int main(int argc, char **argv) {
  Options o;
  const char *error = NULL;
  if (!parse_args(argc, argv, &o, &error)) {
    fprintf(stderr, "Die vergessenen Pfade fehlgeschlagen: %s\n", error);
    return usage();
  }
  int result = 1;
  const char *reason = "Aufbau fehlgeschlagen";
  FILE *log = NULL;
  /* Alles, was cleanup: anfasst, steht vor dem ersten goto: ein Sprung ueber
   * eine Initialisierung hinweg laesst die Variable unbestimmt, und cleanup
   * gibt sie frei. */
  SDL_Window *w = NULL;
  SDL_Renderer *sdl = NULL;
  Renderer r = {0};
  Audio audio = {0};
  const char *base = NULL;
  char assets[1024], verify_reason[96];
  Game g;
  if (o.log_path) {
    log = fopen(o.log_path, "w");
    if (!log) {
      reason = "Protokolldatei kann nicht geschrieben werden";
      goto cleanup;
    }
    fprintf(log, "# time_ms\tmap\tx,y\tevent\tdetails\n");
  }
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    reason = SDL_GetError();
    goto cleanup;
  }
  if (!SDL_CreateWindowAndRenderer("Die vergessenen Pfade | POC Walddorf", 320 * o.scale,
                                   200 * o.scale, SDL_WINDOW_RESIZABLE, &w, &sdl)) {
    reason = SDL_GetError();
    goto cleanup;
  }
  if (o.fullscreen)
    SDL_SetWindowFullscreen(w, true);
  SDL_SetWindowMinimumSize(w, 320, 200);
  if (!SDL_SetRenderLogicalPresentation(sdl, 320, 200,
                                        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
    reason = SDL_GetError();
    goto cleanup;
  }
  base = SDL_GetBasePath();
  if (!base || snprintf(assets, sizeof assets, "%sassets", base) >= (int)sizeof assets) {
    reason = "Basispfad fehlt";
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Die vergessenen Pfade",
                             "Spieldaten fehlen. Der Ordner assets muss neben dem "
                             "Programm liegen. Bitte das Spiel vollstaendig "
                             "entpacken.",
                             w);
    goto cleanup;
  }
  if (!game_init(&g, assets) || !renderer_init(&r, sdl, assets)) {
    reason = "Spieldaten fehlen. Der Ordner assets muss neben dem Programm liegen.";
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR, "Die vergessenen Pfade",
        "Spieldaten fehlen. Der Ordner assets muss neben dem Programm liegen. Bitte "
        "das Spiel vollstaendig entpacken.",
        w);
    goto cleanup;
  }
  if (o.verify) {
    result = run_verify(&r, log, assets, &g, verify_reason, sizeof verify_reason) ? 0 : 1;
    if (result)
      reason = verify_reason;
    goto cleanup;
  }
  result = run_loop(&r, sdl, w, assets, &g, &audio, log, &o, &reason) ? 0 : 1;
cleanup:
  if (result)
    fprintf(stderr, "Die vergessenen Pfade fehlgeschlagen: %s\n", reason);
  audio_close(&audio);
  if (log)
    fclose(log);
  renderer_destroy(&r);
  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(w);
  SDL_Quit();
  return result;
}
