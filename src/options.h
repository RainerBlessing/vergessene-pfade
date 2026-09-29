#ifndef OPTIONS_H
#define OPTIONS_H
#include <stdbool.h>
typedef struct {
  bool smoke, verify, fullscreen;
  int scale;
  const char *log_path;
} Options;
/* The picture is 320x200; the window is a whole multiple of it, so pixels stay
 * square. Two is the smallest that is still comfortable to read. */
#define SCALE_MIN 2
#define SCALE_MAX 5
/* The command line stands alone, so it can be checked before anything runs.
 * On failure *error names the argument problem. */
bool parse_args(int argc, char **argv, Options *o, const char **error);
#endif
