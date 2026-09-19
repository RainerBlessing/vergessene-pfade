#include "audio.h"
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
    case EV_STEP:
      break; /* steps are for the ear, not for the log */
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
/* One confirmation sounds once: the specific sounds come from the events. */
static void act(Game *g, Action a, FILE *log, Audio *audio) {
  bool answering = g->state != GAME_EXPLORATION && (a == ACT_CONFIRM || a == ACT_CANCEL);
  game_action(g, a);
  if (audio && answering)
    audio_play(audio, SFX_CLICK);
  drain_events(g, log, SDL_GetTicks(), audio);
}
static int usage(void) {
  fprintf(stderr, "Aufruf: vergessene_pfade [--smoke | --verify] [--log DATEI]\n");
  return 2;
}
int main(int argc, char **argv) {
  bool smoke = false, verify = false;
  const char *log_path = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--smoke") == 0)
      smoke = true;
    else if (strcmp(argv[i], "--verify") == 0)
      verify = true;
    else if (strcmp(argv[i], "--log") == 0 && i + 1 < argc)
      log_path = argv[++i];
    else
      return usage();
  }
  FILE *log = NULL;
  if (log_path) {
    log = fopen(log_path, "w");
    if (!log) {
      fprintf(stderr, "Protokoll %s kann nicht geschrieben werden.\n", log_path);
      return 1;
    }
    fprintf(log, "# time_ms\tmap\tx,y\tevent\tdetails\n");
  }
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    fprintf(stderr, "%s\n", SDL_GetError());
    if (log)
      fclose(log);
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
  const char *base = SDL_GetBasePath();
  char assets[1024];
  Game g;
  if (!base || snprintf(assets, sizeof assets, "%sassets", base) >= (int)sizeof assets ||
      !game_init(&g, assets) || !renderer_init(&r, sdl, assets)) {
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR, "Die vergessenen Pfade",
        "Spieldaten fehlen. Der Ordner assets muss neben dem Programm liegen. Bitte das "
        "Spiel vollstaendig entpacken.",
        w);
    goto cleanup;
  }
  if (verify) {
    Capture c = {&r, log, true};
    /* One run per journey; each starts from a fresh game. */
    static const struct {
      const char *name;
      bool (*run)(Game *, JourneyObserver, void *);
    } runs[] = {{"exploration", journey},
                {"fight", journey_fight},
                {"boundary", journey_boundary},
                {"mend", journey_mend}};
    bool ok = true;
    for (size_t i = 0; ok && i < sizeof runs / sizeof runs[0]; i++) {
      if (log)
        fprintf(log, "# run\t%s\n", runs[i].name);
      ok = game_init(&g, assets) && runs[i].run(&g, capture, &c);
      drain_events(&g, log, SDL_GetTicks(), NULL); /* before game_init clears them */
    }
    result = ok && c.ok ? 0 : 1;
    goto cleanup;
  }
  Audio audio;
  audio_open(&audio);
  bool run = true;
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
        if (e.key.key == SDLK_F3) {
          audio_cycle(&audio);
          audio_play(&audio, SFX_CLICK);
        } else if (g.state == GAME_NOTEBOOK && e.key.key == SDLK_Q)
          run = false;
        else if (g.state == GAME_NOTEBOOK && e.key.key == SDLK_R) {
          if (!game_init(&g, assets)) {
            result = 1;
            run = false;
          }
        } else {
          act(&g, key(e.key.key), log, &audio);
          last_move = frame_start;
        }
      }
    }
    if (g.state == GAME_EXPLORATION && (SDL_GetWindowFlags(w) & SDL_WINDOW_INPUT_FOCUS) &&
        frame_start - last_move >= 140) {
      Action a = held();
      if (a != ACT_NONE) {
        act(&g, a, log, &audio);
        last_move = frame_start;
      }
    }
    if (frame_start - fps_time >= 1000) {
      fps = (int)(fps_frames * 1000 / (frame_start - fps_time));
      fps_frames = 0;
      fps_time = frame_start;
    }
    fps_frames++;
    audio_update(&audio);
    r.audio = audio_label(&audio);
    render_game(&r, &g, fps);
    if (smoke && ++frames == 10) {
      Capture c = {&r, log, true};
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
  audio_close(&audio);
  if (result)
    fprintf(stderr, "Die vergessenen Pfade fehlgeschlagen: %s\n", SDL_GetError());
  if (log)
    fclose(log);
  renderer_destroy(&r);
  SDL_DestroyRenderer(sdl);
  SDL_DestroyWindow(w);
  SDL_Quit();
  return result;
}
