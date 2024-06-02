/*

The Programmable Interrupt Timer  (PIT or System Clock), is a chip used for
accurately generating interrupts at regular time intervals.
Channel 0 on the PIT is tied ot IRQ0.

The default frequency of the PIT is 18.22 Hz.

*/

#include "include/idt.h"
#include "include/irq.h"
#include "include/process.h"

#define DEF_FREQ 18

int sys_uptime_counter; // Count of the number of ticks from the PIT

/* Timer IRQ handler */
void timer_handler(CpuContext *context) {
  sys_uptime_counter++;
  schedule(context); // The core of the scheduling algorithm
}

/* Add the timer handler to the IRQ based interrupt mapping */
void timer_install() {
  irq_install_handler(0, timer_handler);
  sys_uptime_counter = 0;
}

/* This function can be used to halt the CPU for a specified number of seconds
 */
void timer_wait(double secs) {
  double target = sys_uptime_counter + secs * DEF_FREQ;
  while (sys_uptime_counter < target) {
  }
}
