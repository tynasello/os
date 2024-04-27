#ifndef __ISR_H
#define __ISR_H

#include "idt.h"

void isrs_init();
void isr_install_handler(int isr_no, void (*handler)(CpuContext *context));

#endif
