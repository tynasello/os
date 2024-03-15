#ifndef __IRQ_H
#define __IRQ_H

#include "idt.h"

void irqs_init();
void irq_install_handler(int irq_no, void (*handler)(struct regs *r));

#endif
