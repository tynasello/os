/*
--------------------- 

Interrupt Request

--------------------- 
*/ 

#include "include/io.h"
#include "include/idt.h"
#include "stdint.h"

/*
 
The IRQ based ISRs defined in kernel_entry.asm.

*/
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/*
 
I/O ports for the Programmable Interrupt Controllers (PIC or 8259).
The function of the 8259A is to manage hardware interrupts and send them to the appropriate system interrupt.
Two PIC chips exist for managing IRQs: a master, and a slave. 
The slave controller is connected to IRQ2 of the master. 
The master controller is connected directly to the processor to send signals.

*/
#define MASTER_CMD_PORT 0x20
#define MASTER_DATA_PORT 0x21
#define SLAVE_CMD_PORT 0xA0
#define SLAVE_DATA_PORT 0xA1

/*
 
PIC commands:

End of interrupt (EOI) command used to notify PICs when interrupts have been serviced.

Initialize command makes the PIC wait for 3 extra "initialisation words" on
the data port. These bytes give the PIC: 1) its vector offset, 2) tell it how
to wire master/slaves, 3) gives additional information about the environment.

PIC will add the apporpriate offset to the internal IRQ number to get the correct system interrupt number.

Additional info about environment (use 8086 mode instead of 8080)

*/
#define EOI 0x20
#define INIT 0x11
#define MASTER_OFFSET 0x20
#define SLAVE_OFFSET 0x28
#define PIC_ENV 0x01

/*
 
Remap the PIC IRQ numbers to 32-47.
The PIC initially maps IRQs 0-7 to interrupt numbers 8-15 and IRQs 8-15 to interrupt numbers 112-120.
This will cause conflicts since our IDT already has interrupt numebers 0-31 filled for exception based interrupts. 
The only way to change the vector offsets used by the PIC is to re-initialize it.

*/
void pic_remap() {
  port_byte_out(MASTER_CMD_PORT, INIT);
  port_byte_out(SLAVE_CMD_PORT, INIT);
  port_byte_out(MASTER_DATA_PORT, MASTER_OFFSET);  // Change IRQ numbers of the PIC
  port_byte_out(SLAVE_DATA_PORT, SLAVE_OFFSET);    // ^
  port_byte_out(MASTER_DATA_PORT, 0x04);           // Notify master that slave is at IRQ2
  port_byte_out(SLAVE_DATA_PORT, 0x02);            // Notify slave of its cascade identity
  port_byte_out(MASTER_DATA_PORT, PIC_ENV);
  port_byte_out(SLAVE_DATA_PORT, PIC_ENV);
  port_byte_out(MASTER_DATA_PORT, 0x0);            // Clear controller interrupt masks
  port_byte_out(SLAVE_DATA_PORT, 0x0);             //  ^(all IRQs will be serviced)
}

/*
 
Initialize IRQ based IRSs:
1) remap the PIC to use interrups numbers 32-47
2) set entries 32-47 in the IDT to IRQ based ISRs 0-15.

*/
void irqs_init() {
  pic_remap();

  unsigned short ds = 0x08;
  unsigned short common_flags = 0x8E; // Same flags used for ISR handlers 0-31

  idt_set_entry(32, (uintptr_t)irq0, ds, common_flags);
  idt_set_entry(33, (uintptr_t)irq1, ds, common_flags);
  idt_set_entry(34, (uintptr_t)irq2, ds, common_flags);
  idt_set_entry(35, (uintptr_t)irq3, ds, common_flags);
  idt_set_entry(36, (uintptr_t)irq4, ds, common_flags);
  idt_set_entry(37, (uintptr_t)irq5, ds, common_flags);
  idt_set_entry(38, (uintptr_t)irq6, ds, common_flags);
  idt_set_entry(39, (uintptr_t)irq7, ds, common_flags);
  idt_set_entry(40, (uintptr_t)irq8, ds, common_flags);
  idt_set_entry(41, (uintptr_t)irq9, ds, common_flags);
  idt_set_entry(42, (uintptr_t)irq10, ds, common_flags);
  idt_set_entry(43, (uintptr_t)irq11, ds, common_flags);
  idt_set_entry(44, (uintptr_t)irq12, ds, common_flags);
  idt_set_entry(45, (uintptr_t)irq13, ds, common_flags);
  idt_set_entry(46, (uintptr_t)irq14, ds, common_flags);
  idt_set_entry(47, (uintptr_t)irq15, ds, common_flags);
}

/*
 
An array of function pointers mapping IRQ numbers to handler functions

*/
void *irq_handlers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void irq_install_handler(int irq_no, void (*handler)(CpuContext *context)) {
  if (irq_no >= 0 && irq_no <= 15) { // There are only 16 different IRQs
    irq_handlers[irq_no] = handler;
  }
}

void irq_handler(CpuContext *context) {
  void (*handler)(CpuContext *context); // Blank function pointer
  handler = irq_handlers[context->int_no - 32]; 
  if (handler) {
    handler(context);
  }
  /*
  The CPU tells the PIC that the interrupt is complete by writing an EOI byte to the command port. 
  In the case that the interrupt number is greater than 40 (IRQ 8 or higher), then the slave must
  be notified as well as the master.
  */
  if (context->int_no >= 40) {
    port_byte_out(SLAVE_CMD_PORT, EOI);
  }
  port_byte_out(MASTER_CMD_PORT, EOI);
}
