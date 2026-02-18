#include "uevloop/system/containers/application.h"
#include <ncurses.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

#define NUM_LIGHTS 5

mtx_t uel_critical_section;

struct state {
  uel_application_t app;
  uel_event_t *blink_timer;
  uint8_t current;
  bool light_on;
  volatile atomic_bool running;
};

static void set_blink_timer(struct state *);

inline uint64_t get_time() {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int timer_isr(void *context) {
  struct state *state = context;
  const uint64_t start = get_time();

  while (atomic_load(&state->running)) {
    const uint32_t delta = get_time() - start;
    uel_app_update_timer(&((struct state *)state)->app, delta);
    thrd_sleep(&(struct timespec){.tv_nsec = 1000000}, NULL);
  }

  return EXIT_SUCCESS;
}

enum keypress : uintptr_t { LEFT, RIGHT };

static void *on_keypress(void *context, void *key) {
  struct state *state = context;
  switch ((enum keypress)key) {
  case LEFT:
    state->current = state->current > 0 ? state->current - 1 : NUM_LIGHTS - 1;
    break;
  case RIGHT:
    state->current = state->current < NUM_LIGHTS - 1 ? state->current + 1 : 0;
    break;
  }

  set_blink_timer(state);
  return NULL;
}

static int change_notification_isr(void *context) {
  struct state *state = context;
  uel_closure_t process_keypress = uel_closure_create(&on_keypress, context);

  initscr();
  noecho();
  keypad(stdscr, TRUE);

  while (atomic_load(&state->running)) {
    int ch;
    switch (ch = getch()) {
    case KEY_LEFT:
      uel_app_enqueue_closure(&state->app, &process_keypress, (void *)LEFT);
      break;
    case KEY_RIGHT:
      uel_app_enqueue_closure(&state->app, &process_keypress, (void *)RIGHT);
      break;
    case 'q':
      atomic_store(&state->running, false);
      break;
    }
  }

  endwin();
  return EXIT_SUCCESS;
}

static void *blinker(void *context, void *_data) {
  struct state *state = context;

  clear();

  for (uint8_t i = 0; i < NUM_LIGHTS; i++) {
    if (i == state->current && state->light_on) {
      printw("(*)");
    } else {
      printw("( )");
    }
  }

  refresh();

  state->light_on = !state->light_on;

  return NULL;
}

static void set_blink_timer(struct state *state) {
  if (state->blink_timer) {
    uel_event_timer_cancel(state->blink_timer);
  }

  state->light_on = true;
  state->blink_timer = uel_app_run_at_intervals(
      &state->app, 500, true, uel_closure_create(&blinker, state), NULL);
}

int main(int argc, const char *argv[]) {
  mtx_init(&uel_critical_section, mtx_plain);
  struct state state = {
      .blink_timer = NULL, .current = 0, .light_on = true, .running = true};
  uel_app_init(&state.app);

  thrd_t timer_isr_thread;
  thrd_create(&timer_isr_thread, &timer_isr, &state);

  thrd_t change_notification_isr_thread;
  thrd_create(&change_notification_isr_thread, &change_notification_isr,
              &state);

  set_blink_timer(&state);

  while (atomic_load(&state.running)) {
    uel_app_tick(&state.app);
  }

  thrd_join(timer_isr_thread, NULL);
  thrd_join(change_notification_isr_thread, NULL);

  return 0;
}
