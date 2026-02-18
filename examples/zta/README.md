# Zero to Async side project implementation

This is an implementation of the blinky demo proposed by [this repository](https://github.com/therustybits/zero-to-async)
as requested by [f-zl](https://github.com/f-zl).

## Application description

The original project is an embedded application with two buttons (left and right)
and five horizontally arranged LEDs.
All the time, a single LED is set to a blinky state, with a period of 1s. Any
time a button is pressed, depending on the button, the previous or next LED is
selected as the blinky one, wrapping around the row boundaries.
Additionally, each time a new LED is selected, the blink period should be reset,
meaning it will light immediately upon selection.

## Implementation details

Unlike the original application, this example is not an embedded application, but
actually a console application.
It uses `ncurses` to provide a simple TUI in which each LED is emulated using
ASCII art. The left and right arrow keys are used to emulate the two buttons.

Interruptions are emulated with threads:

- 1khz timer ISR is implemented as a continuously running thread sleeping every
  1ms, while updating the system counter
- a change notification ISR for the buttons is implemented as a thread
  continuously reading user input and dispatching actions

Also, because there is no interruptions to disable in critical sections, the
example instead uses `UEL_CRITICAL_SECTION_OBJ_TYPE`, `UEL_CRITICAL_ENTER`
and `UEL_CRITICAL_EXIT` macros to expose a global mutex as a synchronisation
primitive. They are defined in the `Makefile` so no sources must be edited.

## Library features presented

This example uses the following library features:

- Application creation via `uel_application_t` container
- Scheduling and cancellation of recurring timer events
- Task offloading from ISR/worker threads to the main thread

## Building and running

The only external dependency for this example is a system-wide `libncurses`.

```Makefile
# in examples/zta

make
LD_LIBRARY_PATH=../../dist ./zta
```

### Controls

`left arrow` / `right arrow`: select previous / next LED
`q`: exit
