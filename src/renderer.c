#include "renderer.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static void color(Renderer *r, int c) {
  static const Uint8 colors[][3] = {{24, 25, 34},    {234, 219, 176}, {247, 238, 210},
                                    {183, 63, 52},   {177, 161, 105}, {55, 85, 56},
                                    {234, 219, 176}, {88, 57, 46}};
  if (r->paper) {
    if (c == 1)
      c = 3;
    else if (c == 2)
      c = 0;
    else if (c == 3)
      c = 7;
  }
  SDL_SetRenderDrawColor(r->sdl, colors[c][0], colors[c][1], colors[c][2], 255);
}
static void box(Renderer *r, int x, int y, int w, int h, int c) {
  color(r, c);
  SDL_FRect a = {(float)x, (float)y, (float)w, (float)h};
  SDL_RenderFillRect(r->sdl, &a);
}
/* SDL's built-in bitmap font draws one line; split pages here. */
static void text(Renderer *r, int x, int y, int c, const char *s) {
  color(r, c);
  while (*s) {
    char line[128];
    size_t n = 0;
    while (s[n] && s[n] != '\n' && n < sizeof line - 1)
      n++;
    memcpy(line, s, n);
    line[n] = 0;
    SDL_RenderDebugText(r->sdl, (float)x, (float)y, line);
    s += n;
    if (*s == '\n')
      s++;
    y += 12;
  }
}
static void formatted(Renderer *r, int x, int y, const char *format, ...) {
  char buffer[192];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof buffer, format, args);
  va_end(args);
  SDL_RenderDebugText(r->sdl, (float)x, (float)y, buffer);
}
static void panel(Renderer *r, int x, int y, int w, int h) {
  box(r, x + 2, y + 2, w, h, 0);
  box(r, x, y, w, h, 1);
  box(r, x + 1, y + 1, w - 2, h - 2, 6);
  r->paper = true;
  box(r, x, y + 7, 2, h - 14, 7);
  box(r, x + w - 2, y + 7, 2, h - 14, 7);
  box(r, x + 3, y + 3, w - 6, 1, 3);
}
static void sprite(Renderer *r, int id, int x, int y, int scale) {
  SDL_FRect src = {(float)(id * 16), 0, 16, 16},
            dst = {(float)x, (float)y, (float)(16 * scale), (float)(16 * scale)};
  SDL_RenderTexture(r->sdl, r->atlas, &src, &dst);
}
bool renderer_init(Renderer *r, SDL_Renderer *sdl, const char *assets) {
  r->sdl = sdl;
  r->atlas = NULL;
  char path[1024];
  snprintf(path, sizeof path, "%s/tiles/atlas.bmp", assets);
  SDL_Surface *s = SDL_LoadBMP(path);
  if (!s)
    return false;
  SDL_SetSurfaceColorKey(s, true, SDL_MapSurfaceRGB(s, 255, 0, 255));
  r->atlas = SDL_CreateTextureFromSurface(sdl, s);
  SDL_DestroySurface(s);
  if (!r->atlas)
    return false;
  return SDL_SetTextureScaleMode(r->atlas, SDL_SCALEMODE_NEAREST);
}
void renderer_destroy(Renderer *r) { SDL_DestroyTexture(r->atlas); }
static void world(Renderer *r, const Game *g) {
  int cx, cy;
  game_camera(g, &cx, &cy);
  for (int y = 0; y < 10; y++)
    for (int x = 0; x < 20; x++) {
      char t = map_at(&g->maps[g->map], x + cx, y + cy);
      static const char symbols[] = ".,~T^#H_+><*SBr=Rls[]";
      static const int art[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
                                11, 19, 20, 21, 22, 23, 24, 25, 26, 27};
      const char *at = strchr(symbols, t);
      int id = at ? art[at - symbols] : 4;
      sprite(r, id, x * 16, 16 + y * 16, 1);
      if (g->collision && !map_passable(&g->maps[g->map], x + cx, y + cy))
        box(r, x * 16 + 6, y * 16 + 22, 4, 4, 4);
    }
  SDL_Rect clip = {0, 16, 320, 160};
  SDL_SetRenderClipRect(r->sdl, &clip);
  if (g->map == beast.map && g->quest < OBJECTIVE_FOUND)
    sprite(r, 17, (beast.x - cx) * 16, 16 + (beast.y - cy) * 16, 1);
  for (int i = 0; i < 4; i++)
    if (npcs[i].map == g->map)
      sprite(r, 13 + npcs[i].sprite, (npcs[i].x - cx) * 16, 16 + (npcs[i].y - cy) * 16,
             1);
  sprite(r, 12, (g->x - cx) * 16, 16 + (g->y - cy) * 16, 1);
  box(r, (g->x - cx) * 16 + 7 + g->dx * 6, 16 + (g->y - cy) * 16 + 7 + g->dy * 6, 2, 2,
      2);
  SDL_SetRenderClipRect(r->sdl, NULL);
  box(r, 0, 0, 320, 16, 0);
  text(r, 8, 4, 1,
       g->map == 0 ? "EMBERPOST / DER LATERNENPFAD" : "EMBERPOST / DORF KOHARU");
  box(r, 0, 176, 320, 24, 0);
  color(r, 1);
  formatted(r, 8, 178, "LP %02d/%02d  GOLD %02d  I INVENTAR", g->player.hp,
            g->player.max_hp, g->player.gold);
  int n = game_npc_at(g, g->x + g->dx, g->y + g->dy);
  if (n >= 0)
    text(r, 8, 190, 2, "ENTER REDEN / ESC TAGEBUCH");
  else if (g->message[0])
    text(r, 8, 190, 2, g->message);
  else
    text(r, 8, 190, 2,
         g->quest == NOT_STARTED && g->map == 0 ? "FOLGE DEM WEG NACH NORDEN INS DORF"
                                                : quest_labels[g->quest]);
}
void render_game(Renderer *r, const Game *g, int fps) {
  r->paper = false;
  color(r, 0);
  SDL_RenderClear(r->sdl);
  world(r, g);
  if (g->state == GAME_DIALOGUE) {
    panel(r, 4, 105, 312, 91);
    text(r, 12, 114, 1, npcs[g->npc].name);
    text(r, 12, 132, 2, dialogues[g->dialogue].pages[g->page]);
    color(r, 3);
    formatted(r, 12, 181, "ENTER WEITER %d/%d  ESC SCHLIESSEN", g->page + 1,
              dialogues[g->dialogue].count);
  }
  if (g->state == GAME_INVENTORY || g->state == GAME_SHOP) {
    panel(r, 4, 26, 312, 146);
    color(r, 1);
    formatted(r, 12, 36, "%s     GOLD %d",
              g->state == GAME_SHOP ? "RENS LADEN" : "DEIN INVENTAR", g->player.gold);
    int count = g->state == GAME_SHOP ? 2 : ITEM_COUNT;
    for (int i = 0; i < count; i++) {
      ItemId id = g->state == GAME_SHOP ? shop_stock[i] : (ItemId)i;
      if (g->selection == i)
        box(r, 10, 55 + i * 16, 300, 14, 4);
      color(r, g->selection == i ? 1 : 2);
      formatted(r, 14, 58 + i * 16, "%c %-15s %2d %s", g->selection == i ? '>' : ' ',
                items[id].name,
                g->state == GAME_SHOP ? items[id].price
                                      : g->player.inventory.quantities[id],
                g->state == GAME_SHOP ? "GOLD" : "STUECK");
    }
    text(r, 12, 115, 2, g->message);
    color(r, 3);
    formatted(r, 12, 137, "ANG %d  ABW %d / TEE HEILT 8 LP", g->player.attack,
              g->player.defense);
    text(r, 12, 157, 1, "HOCH/RUNTER  ENTER WAHL  ESC ENDE");
  }
  if (g->state == GAME_COMBAT) {
    panel(r, 4, 21, 312, 175);
    text(r, 16, 31, 1, "WALDSCHREIN / DORNENGEIST");
    color(r, 2);
    formatted(r, 16, 46, "GEIST %02d/%02d       DU %02d/%02d", g->combat.hp, beast.hp,
              g->player.hp, g->player.max_hp);
    box(r, 16, 57, 128, 3, 3);
    box(r, 16, 57, 128 * g->combat.hp / beast.hp, 3, 4);
    box(r, 176, 57, 128, 3, 3);
    box(r, 176, 57, 128 * g->player.hp / g->player.max_hp, 3, 1);
    sprite(r, g->combat.won ? 18 : 17, 144, 64, 2);
    text(r, 12, 103, 2, g->message);
    if (g->combat.won || g->combat.lost)
      text(r, 12, 177, 1, "ENTER / ZURUECK AUF DEN WEG");
    else {
      box(r, 12, 137 + g->selection * 15, 296, 14, 4);
      color(r, 1);
      formatted(r, 16, 140, "%c ANGREIFEN", g->selection == 0 ? '>' : ' ');
      formatted(r, 16, 155, "%c BEIFUSSTEE (%d)", g->selection == 1 ? '>' : ' ',
                g->player.inventory.quantities[ITEM_HERB]);
      text(r, 12, 180, 2, "HOCH/RUNTER WAHL / ENTER AKTION");
    }
  }
  if (g->state == GAME_PAUSED) {
    panel(r, 4, 22, 312, 174);
    text(r, 16, 33, 1, "EMBERPOST / REISETAGEBUCH");
    text(r, 16, 53, 2, quest_labels[g->quest]);
    text(r, 16, 73, 3,
         "AOI: HAUS IM NORDWESTEN\nREN: LADEN IM NORDOSTEN\nNAO: AM SUEDTOR");
    text(r, 16, 105, 2,
         "PFEILE / WASD  LAUFEN\nLEERTASTE / ENTER  REDEN / WAHL\nI              "
         "INVENTAR\nF1 / F2        DEBUG / HINDERNISSE");
    text(r, 16, 157, 1, "ENTER / ESC WEITER\nR NEUE REISE / Q BEENDEN");
    text(r, 16, 182, 3, "EIN WEG. EIN LICHT. DEINE REISE.");
  }
  if (g->debug) {
    static const char *states[] = {"WELT", "DIALOG", "TASCHE", "LADEN", "KAMPF", "PAUSE"};
    panel(r, 4, 16, 312, 37);
    color(r, 2);
    formatted(r, 10, 23, "KARTE %d XY %d,%d BPS %d", g->map, g->x, g->y, fps);
    formatted(r, 10, 37, "%s AUFTRAG %d FIGUREN %d", states[g->state], g->quest,
              g->map == 1 ? 4 : 0);
  }
}
