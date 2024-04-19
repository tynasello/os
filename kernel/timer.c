/*
--------------------- 

Timer

The Programmable Interrupt Timer  (PIT or System Clock), is a chip used for
accurately generating interrupts at regular time intervals.  
Channel 0 on the PIT is tied ot IRQ0.

The default frequency of the PIT is 18.22 Hz.

--------------------- 
*/

#include "include/screen.h"
#include "include/irq.h"

#define NUM_OBSERVERS 5

int sys_uptime_counter = 0; // Count of the number of ticks from the PIT

/* 
Register observers to call after a specified number of seconds.
Return 0 if registration unsuccessful.
*/
static void* observers[NUM_OBSERVERS];
static int observers_time[NUM_OBSERVERS];

int register_timer_observer(void* fn, int secs){
  int i;
  for(i = 0; i < NUM_OBSERVERS; i++){
    if(observers[i] == 0){
      break;
    }
  }
  if (i >= NUM_OBSERVERS){
    return 0;
  }

  observers[i] = fn;
  observers_time[i] = secs;

  return 1;
}

void deregister_timer_observer(void* fn){
  for(int i = 0; i < NUM_OBSERVERS; i++){
    if(observers[i] == fn){
      observers[i] = 0;
      observers_time[i] = 0;
    }
  }
}


/*
Timer IRQ handler
*/
void timer_handler(struct regs *r) {
  for (int i = 0; i < NUM_OBSERVERS; i++){
    if(observers[i] == 0){
      break;
    }
    void (*fn)(); 
    fn = observers[i];
    if(observers_time[i] == 0){
      fn();
    } else if (++sys_uptime_counter % (18 * observers_time[i]) == 0) {                            
      fn();
    }
  }
}

/*
Add the timer handler to the IRQ based interrupt mapping
*/
void timer_install() { 
  for (int i = 0; i < NUM_OBSERVERS; i++){
    observers[i] = 0;
  }
  irq_install_handler(0, timer_handler); 
}

/*
This function can be used to halt the CPU for a specified number of seconds
*/
void timer_wait(int secs) {
  unsigned long target = sys_uptime_counter + secs * 18;
  while (sys_uptime_counter < target) {
  }
}
