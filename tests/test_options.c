#include "options.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define CHECK(c)                                                                         \
  do {                                                                                   \
    if (!(c)) {                                                                          \
      fprintf(stderr, "FAIL %s:%d %s\n", __FILE__, __LINE__, #c);                        \
      failures++;                                                                        \
    }                                                                                    \
  } while (0)
static bool parse(Options *o, const char **error, int argc, char **argv) {
  *error = NULL;
  return parse_args(argc, argv, o, error);
}
int main(void) {
  Options o;
  const char *e;
  {
    char *a[] = {"p"};
    CHECK(parse(&o, &e, 1, a));
    CHECK(o.scale == 4);
    CHECK(!o.smoke && !o.verify && !o.fullscreen);
    CHECK(!o.log_path);
  }
  {
    char *a[] = {"p", "--scale", "3", "--fullscreen", "--log", "x.tsv"};
    CHECK(parse(&o, &e, 6, a));
    CHECK(o.scale == 3);
    CHECK(o.fullscreen);
    CHECK(strcmp(o.log_path, "x.tsv") == 0);
  }
  {
    char *a[] = {"p", "--verify"};
    CHECK(parse(&o, &e, 2, a));
    CHECK(o.verify);
  }
  {
    char *a[] = {"p", "--bogus"};
    CHECK(!parse(&o, &e, 2, a));
    CHECK(strstr(e, "Unbekanntes"));
  }
  {
    char *a[] = {"p", "--scale", "9"};
    CHECK(!parse(&o, &e, 3, a));
    CHECK(strstr(e, "Skalierung"));
  }
  {
    char *a[] = {"p", "--scale", "3x"};
    CHECK(!parse(&o, &e, 3, a));
    CHECK(strstr(e, "Skalierung"));
  }
  {
    char *a[] = {"p", "--scale"};
    CHECK(!parse(&o, &e, 2, a));
    CHECK(strstr(e, "--scale braucht"));
  }
  {
    char *a[] = {"p", "--log"};
    CHECK(!parse(&o, &e, 2, a));
    CHECK(strstr(e, "--log braucht"));
  }
  {
    char *a[] = {"p", "--smoke", "--verify"};
    CHECK(!parse(&o, &e, 3, a));
    CHECK(strstr(e, "schliessen"));
  }
  return failures ? 1 : 0;
}
