#include "include/screen.h"
#include "include/irq.h"

/*
 
The Programmable Interrupt Timer  (PIT or System Clock), is a chip used for
accurately generating interrupts at regular time intervals.  
Channel 0 on the PIT is tied ot IRQ0.

The default frequency of the PIT is 18.22 Hz.

*/

int sys_uptime_counter = 0; // Count of the number of ticks from the PIT

void timer_handler(struct regs *r) {
  if (++sys_uptime_counter % 18 == 0) {                            
    print("One second has passed\n"); // Approxiamtely*
  }
}

/*
 
Add the timer handler to the IRQ based interrupt mapping

*/
void timer_install() { 
  // irq_install_handler(0, timer_handler); 
}

/*
 
This function can be used to halt the CPU for a specified number of seconds

*/
void timer_wait(int secs) {
  unsigned long target = sys_uptime_counter + secs * 18;
  while (sys_uptime_counter < target) {
    print(""); // Dummy instruction
  }
}
