#include "../drivers/io.h"
#include "include/system.h"

// Prototypes for the frist 16 IRQ based ISRs defined in kernel_entry.asm.
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

// I/O ports for the Programmable Interrupt Controllers (PIC or 8259)
// The function of the 8259A is to manage hardware interrupts and send them to
// the appropriate system interrupt. Without a PIC, you would have to poll all
// the devices in the system to see if they want to do anything (signal an
// event), but with a PIC, your system can run along nicely until such time that
// a device wants to signal an event.
// Two PIC chips exist for managing IRQs, one is a master controller, and the
// other is a slave PIC connected to IRQ2 on the master. The master controller
// is connected directly to the processor to send signals.

// I/O control port for the master PIC
#define MASTER_CMD 0x20
// I/O data port for the master PIC
#define MASTER_DATA 0x21
#define SLAVE_CMD 0xA0
#define SLAVE_DATA 0xA1

// PIC commands:
// End of interrupt (EOI) command used to notify PICs when interrupts have been
// serviced
#define EOI 0x20
// Initialize command makes the PIC wait for 3 extra "initialisation words" on
// the data port. These bytes give the PIC: 1) its vector offset, 2) tell it how
// to wire master/slaves, 3) gives additional information about the environment
#define INIT 0x11
// PIC will add the offset to the internal IRQ number to get the correct system
// number
#define MASTER_OFFSET 0x20
#define SLAVE_OFFSET 0x28
// Additional info about environment (use 8086 mode instead of 8080)
#define PIC_ENV 0x01

// The PIC initially maps IRQs 0-7 to interrupt numbers 8-15 and IRQs 8-15 to
// interrupt numbers 112-120. This could cause conflicts, and so will be
// remapped to interrupt numbers 32-47, just after the first 32 exception based
// interrupts. The only way to change the vector offsets used by the 8259 is
// to re-initialize it.
void pic_remap() {
  port_byte_out(MASTER_CMD, INIT);
  port_byte_out(SLAVE_CMD, INIT);
  port_byte_out(MASTER_DATA, MASTER_OFFSET);
  port_byte_out(SLAVE_DATA, SLAVE_OFFSET);
  port_byte_out(MASTER_DATA, 0x04); // Notify master that slave is at IRQ2
  port_byte_out(SLAVE_DATA, 0x02);  // Notify slave of its cascade identity
  port_byte_out(MASTER_DATA, PIC_ENV);
  port_byte_out(SLAVE_DATA, PIC_ENV);
  port_byte_out(MASTER_DATA, 0x0); // Clear controller interrupt masks
  port_byte_out(SLAVE_DATA, 0x0);  //   (all IRQs will be serviced)
}

// Initialize IRQ based IRSs:
// 1) remap the PIC to use interrups numbers 32-47
// 2) set entries 32-47 in the IDT to IRQ based ISRs 0-15.
void irqs_init() {
  pic_remap();

  unsigned short ds = 0x08;
  unsigned short common_flags = 0x8E; // Same flags used for ISR handlers 0-31

  idt_set_entry(32, (unsigned int)irq0, ds, common_flags);
  idt_set_entry(33, (unsigned int)irq1, ds, common_flags);
  idt_set_entry(34, (unsigned int)irq2, ds, common_flags);
  idt_set_entry(35, (unsigned int)irq3, ds, common_flags);
  idt_set_entry(36, (unsigned int)irq4, ds, common_flags);
  idt_set_entry(37, (unsigned int)irq5, ds, common_flags);
  idt_set_entry(38, (unsigned int)irq6, ds, common_flags);
  idt_set_entry(39, (unsigned int)irq7, ds, common_flags);
  idt_set_entry(40, (unsigned int)irq8, ds, common_flags);
  idt_set_entry(41, (unsigned int)irq9, ds, common_flags);
  idt_set_entry(42, (unsigned int)irq10, ds, common_flags);
  idt_set_entry(43, (unsigned int)irq11, ds, common_flags);
  idt_set_entry(44, (unsigned int)irq12, ds, common_flags);
  idt_set_entry(45, (unsigned int)irq13, ds, common_flags);
  idt_set_entry(46, (unsigned int)irq14, ds, common_flags);
  idt_set_entry(47, (unsigned int)irq15, ds, common_flags);
}

// Array of function pointers mapping IRQ number to custom IRQ handler functions
void *irq_handlers[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void irq_install_handler(int irq_no, void (*handler)(struct regs *r)) {
  if (irq_no >= 0 && irq_no <= 15) {
    irq_handlers[irq_no] = handler;
  }
}

void irq_handler(struct regs *r) {

  void (*handler)(struct regs *r); // Blank fn ptr

  // Run the custom handler for this IRQ if it has been defined
  handler = irq_handlers[r->int_no - 32];
  if (handler) {
    handler(r);
  }

  // The CPU tells the right PIC that the interrupt is complete by writing the
  // byte 0x20 (EOI) to the command port for that PIC. In the case that the
  // interrupt number is greater than 40 (IRQ 8 or higher), then the slave must
  // be notified as well as the master.
  if (r->int_no >= 40) {
    port_byte_out(SLAVE_CMD, EOI);
  }
  port_byte_out(MASTER_CMD, EOI);
}
