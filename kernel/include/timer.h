#ifndef __TIMER_H
#define __TIMER_H

void timer_install();
void timer_wait(int secs);
int register_timer_observer(void* fn, int secs);
void deregister_timer_observer(void* fn);

#endif 
