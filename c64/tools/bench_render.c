/* Was ein Bildaufbau kostet -- gemessen auf der emulierten Maschine.
 *
 * Statt main.c aus dem Spiel: dieselben Quellen, aber ohne Tastatur. Das
 * Programm stellt das Spiel auf Erkundung, zeichnet ROUNDS Bilder mit jeweils
 * neuer Spielerposition (damit map_is_current() den Kartenteil wirklich neu
 * zeichnet) und zaehlt die Takte dazwischen mit den Timern von CIA#2.
 *
 * Das Ergebnis liegt danach in bench_cycles/bench_overhead; tools/bench.py
 * holt es ueber den VICE-Monitor ab. Der Kernal-Interrupt ist waehrend der
 * Messung abgeschaltet, gemessen wird also der Code, nicht die Tastaturabfrage.
 * Die Badlines des VIC stecken mit drin -- sie gehoeren zum Bildaufbau auf der
 * echten Maschine und treffen jede Uebersetzung gleich. */
#include "game.h"
#include "render.h"
#include "screen.h"
#include <stdint.h>

#define ROUNDS 8

#define CIA2_TIMERS ((volatile uint8_t *)0xdd04) /* TA lo/hi, TB lo/hi */
#define CIA2_CRA (*(volatile uint8_t *)0xdd0e)
#define CIA2_CRB (*(volatile uint8_t *)0xdd0f)
#define CIA1_ICR (*(volatile uint8_t *)0xdc0d)

/* Was bench.py abholt. bench_ready kommt zuletzt, damit der Monitor nie halbe
 * Zahlen liest. */
volatile uint32_t bench_cycles;
volatile uint32_t bench_overhead;
volatile uint8_t bench_rounds;
volatile uint8_t bench_ready;

static void timer_start(void) {
  CIA2_TIMERS[0] = 0xff; /* Timer A zaehlt Systemtakte */
  CIA2_TIMERS[1] = 0xff;
  CIA2_TIMERS[2] = 0xff; /* Timer B zaehlt die Ueberlaeufe von Timer A */
  CIA2_TIMERS[3] = 0xff;
  CIA2_CRB = 0x51; /* laden, starten, Eingang: Ueberlauf Timer A */
  CIA2_CRA = 0x11; /* laden, starten, Eingang: Systemtakt */
}

/* Beide Timer laufen abwaerts; zusammen sind sie ein 32-Bit-Taktzaehler.
 * Zweimal Timer B lesen faengt den Ueberlauf zwischen den Lesevorgaengen. */
static uint32_t timer_read(void) {
  uint16_t high, low, again;
  do {
    high = (uint16_t)(CIA2_TIMERS[2] | (CIA2_TIMERS[3] << 8));
    low = (uint16_t)(CIA2_TIMERS[0] | (CIA2_TIMERS[1] << 8));
    again = (uint16_t)(CIA2_TIMERS[2] | (CIA2_TIMERS[3] << 8));
  } while (high != again);
  CIA2_CRA = 0;
  CIA2_CRB = 0;
  return ((uint32_t)(uint16_t)(0xffffu - high) << 16) | (uint16_t)(0xffffu - low);
}

int main(void) {
  static Game game;
  screen_init();
  game_init(&game);
  /* Titel und Ankunftsszene wegbestaetigen, bis die Karte steht. */
  for (uint8_t guard = 20; guard && game.state != GAME_EXPLORATION; guard--)
    game_action(&game, ACT_CONFIRM);
  render(&game); /* einmal warmlaufen, damit nur noch gezeichnet wird */

  CIA1_ICR = 0x7f; /* Kernal-Interrupt aus */
  timer_start();
  bench_overhead = timer_read(); /* leeres Fenster: was das Messen selbst kostet */
  timer_start();
  for (uint8_t i = 0; i < ROUNDS; i++) {
    game.x ^= 1; /* eine Zelle hin und her: jedes Mal ein neuer Kartenteil */
    render(&game);
  }
  uint32_t total = timer_read();
  CIA1_ICR = 0x81; /* Kernal-Interrupt wieder an */

  bench_cycles = total;
  bench_rounds = ROUNDS;
  bench_ready = 0x42;
  for (;;) {
  }
}
