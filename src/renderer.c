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
static int tile_art(char t) {
  static const char symbols[] = ".,~T^#H_+><*SBr=Rls[]GxOoYmdAkWMhFft";
  static const int art[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                            19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                            31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43};
  const char *at = strchr(symbols, t);
  return at ? art[at - symbols] : 4;
}
static void world(Renderer *r, const Game *g) {
  static const int npc_art[NPC_COUNT] = {13, 14, 15, 16, 40};
  int cx, cy;
  game_camera(g, &cx, &cy);
  for (int y = 0; y < 10; y++)
    for (int x = 0; x < 20; x++) {
      sprite(r, tile_art(game_tile(g, g->map, x + cx, y + cy)), x * 16, 16 + y * 16, 1);
      if (g->collision && !game_passable(g, g->map, x + cx, y + cy))
        box(r, x * 16 + 6, y * 16 + 22, 4, 4, 4);
    }
  SDL_Rect clip = {0, 16, 320, 160};
  SDL_SetRenderClipRect(r->sdl, &clip);
  for (int i = 0; i < NPC_COUNT; i++)
    if (npcs[i].map == g->map)
      sprite(r, npc_art[npcs[i].sprite], (npcs[i].x - cx) * 16,
             16 + (npcs[i].y - cy) * 16, 1);
  sprite(r, 12, (g->x - cx) * 16, 16 + (g->y - cy) * 16, 1);
  box(r, (g->x - cx) * 16 + 7 + g->dx * 6, 16 + (g->y - cy) * 16 + 7 + g->dy * 6, 2, 2,
      2);
  SDL_SetRenderClipRect(r->sdl, NULL);
  box(r, 0, 0, 320, 16, 0);
  text(r, 8, 4, 1, g->map == MAP_VILLAGE ? "KIRIYAMA" : "DER WALD");
  box(r, 0, 176, 320, 24, 0);
  color(r, 1);
  formatted(r, 8, 178, "LP %02d/%02d  I TASCHE  ESC NOTIZBUCH", g->player.hp,
            g->player.max_hp);
  /* The hint never depends on whether a tile hides something (no markers). */
  if (g->message[0])
    text(r, 8, 190, 2, g->message);
  else
    text(r, 8, 190, 2,
         game_npc_at(g, g->x + g->dx, g->y + g->dy) >= 0 ? "ENTER REDEN"
                                                         : "ENTER UNTERSUCHEN");
}
static void dialogue_panel(Renderer *r, const Game *g) {
  const TileDef *tile = tile_def(g->examined);
  const char *title = g->npc >= 0               ? npcs[g->npc].name
                      : g->npc == SPEAKER_SCENE ? "Unterwegs"
                      : tile                    ? tile->name
                                                : "In deiner Tasche";
  panel(r, 4, 105, 312, 91);
  text(r, 12, 114, 1, title);
  text(r, 12, 132, 2, dialogues[g->dialogue].pages[g->page]);
  color(r, 3);
  if (dialogues[g->dialogue].count > 1)
    formatted(r, 12, 181, "ENTER WEITER %d/%d  ESC SCHLIESSEN", g->page + 1,
              dialogues[g->dialogue].count);
  else
    formatted(r, 12, 181, "ENTER / ESC SCHLIESSEN");
}
static void inventory_panel(Renderer *r, const Game *g) {
  ItemId owned[ITEM_COUNT];
  int count = game_owned_items(g, owned);
  panel(r, 4, 26, 312, 146);
  text(r, 12, 36, 1, "DEINE TASCHE");
  if (count == 0)
    text(r, 14, 58, 2, "Die Tasche ist leer.");
  for (int i = 0; i < count; i++) {
    if (g->selection == i)
      box(r, 10, 55 + i * 16, 300, 14, 4);
    color(r, g->selection == i ? 1 : 2);
    formatted(r, 14, 58 + i * 16, "%c %-16s %2d", g->selection == i ? '>' : ' ',
              items[owned[i]].name, g->player.inventory.quantities[owned[i]]);
  }
  text(r, 12, 125, 2, g->message);
  text(r, 12, 157, 1, "ENTER ANSEHEN  /  ESC ZURUECK");
}
static void encounter_panel(Renderer *r, const Game *g) {
  int options[ENC_COUNT];
  int count = game_encounter_options(g, options);
  panel(r, 4, 21, 312, 175);
  text(r, 16, 31, 1, "AM RAND DES HAINS");
  color(r, 3);
  formatted(r, 16, 45, "DER KAMI WIRKT %s", mood_names[g->mood]);
  sprite(r, 17, 144, 56, 2);
  text(r, 12, 92, 2, g->message);
  for (int i = 0; i < count; i++) {
    if (g->selection == i)
      box(r, 12, 134 + i * 15, 296, 14, 4);
    color(r, g->selection == i ? 1 : 2);
    EncounterAction action = encounter_options[options[i]].action;
    if (action == ENC_OFFER)
      formatted(r, 16, 137 + i * 15, "%c %s: %s", g->selection == i ? '>' : ' ',
                encounter_options[options[i]].label, items[game_encounter_offer(g)].name);
    else
      formatted(r, 16, 137 + i * 15, "%c %s", g->selection == i ? '>' : ' ',
                encounter_options[options[i]].label);
  }
  text(r, 12, 182, 3, "HOCH/RUNTER WAHL  ENTER HANDELN");
}
static void notebook_panel(Renderer *r, const Game *g) {
  panel(r, 4, 22, 312, 174);
  text(r, 16, 31, 1, "NOTIZBUCH");
  if (g->note_count == 0)
    text(r, 16, 51, 2, "Noch keine Notizen.");
  for (int i = 0; i < NOTES_PER_PAGE && g->scroll + i < g->note_count; i++)
    text(r, 16, 49 + i * 28, 2, notes[g->notes[g->scroll + i]]);
  color(r, 3);
  if (g->scroll > 0)
    formatted(r, 292, 31, "^");
  if (g->scroll + NOTES_PER_PAGE < g->note_count)
    formatted(r, 292, 154, "v");
  text(r, 16, 166, 3, "HOCH/RUNTER BLAETTERN  ESC WEITER");
  text(r, 16, 180, 1, "R NEU BEGINNEN  /  Q BEENDEN");
}
void render_game(Renderer *r, const Game *g, int fps) {
  r->paper = false;
  color(r, 0);
  SDL_RenderClear(r->sdl);
  world(r, g);
  if (g->state == GAME_DIALOGUE)
    dialogue_panel(r, g);
  if (g->state == GAME_INVENTORY)
    inventory_panel(r, g);
  if (g->state == GAME_NOTEBOOK)
    notebook_panel(r, g);
  if (g->state == GAME_ENCOUNTER)
    encounter_panel(r, g);
  if (g->debug) {
    static const char *states[] = {"WELT", "DIALOG", "TASCHE", "NOTIZ", "KAMI"};
    panel(r, 4, 16, 312, 37);
    color(r, 2);
    formatted(r, 10, 23, "KARTE %d XY %d,%d BPS %d", g->map, g->x, g->y, fps);
    formatted(r, 10, 37, "%s BEOB %05X NOTIZEN %d", states[g->state], (unsigned)g->obs,
              g->note_count);
  }
}
