#include "options.h"
#include <stdlib.h>
#include <string.h>
static bool parse_scale(const char *text, int *scale) {
  char *end;
  long v = strtol(text, &end, 10);
  if (end == text || *end != '\0' || v < SCALE_MIN || v > SCALE_MAX)
    return false;
  *scale = (int)v;
  return true;
}
bool parse_args(int argc, char **argv, Options *o, const char **error) {
  o->smoke = o->verify = o->fullscreen = false;
  o->scale = 4;
  o->log_path = NULL;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--smoke") == 0)
      o->smoke = true;
    else if (strcmp(argv[i], "--verify") == 0)
      o->verify = true;
    else if (strcmp(argv[i], "--fullscreen") == 0)
      o->fullscreen = true;
    else if (strcmp(argv[i], "--log") == 0 || strcmp(argv[i], "--scale") == 0) {
      bool is_log = argv[i][2] == 'l';
      if (i + 1 >= argc) {
        *error = is_log ? "--log braucht einen Dateinamen" : "--scale braucht eine Zahl";
        return false;
      }
      if (is_log)
        o->log_path = argv[++i];
      else if (!parse_scale(argv[++i], &o->scale)) {
        *error = "Skalierung muss eine Zahl zwischen 2 und 5 sein";
        return false;
      }
    } else {
      *error = "Unbekanntes Argument";
      return false;
    }
  }
  if (o->smoke && o->verify) {
    *error = "--smoke und --verify schliessen sich aus";
    return false;
  }
  return true;
}
