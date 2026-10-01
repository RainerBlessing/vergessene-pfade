/* What each outcome changes on the map, and the marks and tracks that appear.
 * Shared by PC and C64; each version says what TILE(...) is before it includes
 * tile_changes.inc and tile_overrides.inc: map, x, y, symbol, needs, outcome, phase,
 * tag (the tag is for the PC's balance test; the C64 ignores it). */
#define CHANGE(map, x, y, sym, out, ph, tag) TILE(map, x, y, sym, 0, out, ph, tag)
/* Sobald der Handel steht, sind die drei Stellen zu sehen: Suchen gehoert nicht
 * zu dieser Entscheidung, sie ist laengst gefallen (#21). */
#define MARK(x, y)                                                                       \
  TILE(MAP_FOREST, x, y, 'P', OBS(OBS_DAIGO_DEAL), OUT_ANY, PHASE_ANY, TAG_NEUTRAL)
#define TRACK(x, y)                                                                      \
  TILE(MAP_FOREST, x, y, 't', OBS(OBS_FOX_TENDED), OUT_ANY, PHASE_ANY, TAG_NEUTRAL)
