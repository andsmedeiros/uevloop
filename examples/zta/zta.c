#include "uevloop/system/containers/application.h"
#include <ncurses.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

// The original project implements only 5 LEDs, we could emulate more if desired
#define NUM_LIGHTS 5

// This is the global mutex used for synchronisation.
// It is our duty to declare and initialise it.
mtx_t uel_critical_section;

// Encapsulates all our application's state for easy access
struct state {
  // This is the main applciation container. We don't have to know what is in
  // there.
  uel_application_t app;

  // This will store the event associated with the blinking behaviour.
  // We need to keeps this around because we need to cancel the event when
  // a new LED is selected.
  uel_event_t *blink_timer;

  // Stores the currently blinking LED
  uint8_t current;

  // Stores whether the currently selected light should be light on the next
  // timer event invocation
  bool light_on;

  // A simple flag to exit graciously
  volatile atomic_bool running;
};

static void set_blink_timer(struct state *);

// Gets the current time as number of milliseconds.
// This is a cheap way to keep time, but will have overflow issues eventually.
inline uint64_t get_time() {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Emulates a 1kHz timer ISR.
// Responsible for feeding the system counter.
static int timer_isr(void *context) {
  struct state *state = context;
  const uint64_t start = get_time();

  while (atomic_load(&state->running)) {
    const uint32_t delta = get_time() - start;
    uel_app_update_timer(&((struct state *)state)->app, delta);

    // Sleeps for 1ms aproximately
    thrd_sleep(&(struct timespec){.tv_nsec = 1000000}, NULL);
  }

  return EXIT_SUCCESS;
}

// Defines which inputs we recognise
enum keypress : uintptr_t { LEFT, RIGHT };

// Closure function to process an emulated button press.
// Will update the application's internal state with the selected LED and
// reset the blinking timer event.
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

// Emulates a change notification ISR (this is called differently by each MCU
// manufacturer, but is basically an external interruption).
// On an embedded device, we would react to this interruption by reading the
// buttons' pins and determining which one is pressed before dispatching the
// appropriate action. This is a good example also of how to offload some work
// from ISRs or worker threads to the main thread by calling
// `uel_app_enqueue_closure`.
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

// Closure function to blink the selected LED.
// This is called repeatedly to print the internal state and to toggle
// the currently selected LED.
// A truly embedded application would write the application state to a physical
// port connected to the actual LEDs instead of printing things to the terminal.
static void *blinker(void *context, void *_data) {
  struct state *state = context;

  clear();

  for (uint8_t i = 0; i < NUM_LIGHTS; i++) {
    if (i == state->current && state->light_on) {
      // LED is selected and its time to turn on
      printw("(*)");
    } else {
      // Either the LED is not selected or we are on downtime
      printw("( )");
    }
  }

  refresh();

  // Toggle the LED state.
  // Because our closure is called at regular intervals, this will produce a
  // PWM with period=1s, duty cycle=50%
  state->light_on = !state->light_on;

  return NULL;
}

// Restarts the blink timer event.
// This creates a new recurring event and sets its first execution
// to occurr immediately.
static void set_blink_timer(struct state *state) {
  if (state->blink_timer) {
    uel_event_timer_cancel(state->blink_timer);
  }

  state->light_on = true;
  state->blink_timer = uel_app_run_at_intervals(
      &state->app, 500, true, uel_closure_create(&blinker, state), NULL);
}

int main(int argc, const char *argv[]) {
  // Initialises the global mutex before all
  mtx_init(&uel_critical_section, mtx_plain);

  // Creates application state and initialise the application.
  // From here on, it is ready to be used (i.e.: enqueue and schedule things),
  // but as it is not being fed, anything won't be processed yet.
  struct state state = {
      .blink_timer = NULL, .current = 0, .light_on = true, .running = true};
  uel_app_init(&state.app);

  // Spawns the timer ISR emulator thread
  thrd_t timer_isr_thread;
  thrd_create(&timer_isr_thread, &timer_isr, &state);

  // Spawns the change notification ISR emulator thread
  thrd_t change_notification_isr_thread;
  thrd_create(&change_notification_isr_thread, &change_notification_isr,
              &state);

  // Sets first blink timer instance
  set_blink_timer(&state);

  // Spins to run the application. This is where all work happens.
  // Each `uel_app_tick` call is a run loop.
  while (atomic_load(&state.running)) {
    uel_app_tick(&state.app);
  }

  // Wait for other threads to die peacefully
  thrd_join(timer_isr_thread, NULL);
  thrd_join(change_notification_isr_thread, NULL);

  // Good-bye
  return 0;
}
