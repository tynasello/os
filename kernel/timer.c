#include "include/system.h"
#include "../drivers/screen.h"

// The Programmable Interrupt Timer  (PIT or System Clock), is a chip used for
// accurately generating interrupts at regular time intervals.  Chanel 0 on the
// PIT is tied ot IRQ0.

int sys_uptime_counter = 0; // Count the number of ticks from the PIT

// This function will run whenever the PIT generates a interrupt
void timer_handler(struct regs *r) {
  if (++sys_uptime_counter % 18 == 0) { // The default freq of the PIT is 18.22 Hz
    print("One second has passed\n"); // Approxiamtely*
  }
}

// Install the custom timer handler in our IRQ based interrupt mapping.
void timer_install() { irq_install_handler(0, timer_handler); }

// This function can be used to halt the CPU for a specific number of seconds
void timer_wait(int secs) {
  unsigned long targ_ticks = sys_uptime_counter + secs * 18; 
  while (sys_uptime_counter < targ_ticks) {
    print("");
  }
}
