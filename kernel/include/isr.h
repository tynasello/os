#ifndef __ISR_H
#define __ISR_H

void isrs_init();
void isr_install_handler(int isr_no, void (*handler)(struct regs *r));

#endif
