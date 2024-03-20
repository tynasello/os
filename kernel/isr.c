#include "include/screen.h"
#include "include/idt.h"

/*

The frist 32 ISR defined in kernel_entry.asm.
Interrupt numbers 0-31 are reserved by Intel, and are designed to service exceptions.

*/
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/*
 
Mapping from interrupt number to exception message

*/
char *exception_messages[32];

/*
 
Initialize exception based ISRs:
Set entries 0-32 in the IDT to exception based ISRs.

*/
void isrs_init() {
  unsigned short ds = 0x08;
  unsigned short common_flags = 0x8E; // 10001110: 
                                      // descriptor is valid (bit 7), DPL is ring 0 (bits 5-6),
                                      // bit 4 is always 0, gate type is 32-bit interrupt gate (bits 0-3).

  idt_set_entry(0, (unsigned int)isr0, ds, common_flags);
  idt_set_entry(1, (unsigned int)isr1, ds, common_flags);
  idt_set_entry(2, (unsigned int)isr2, ds, common_flags);
  idt_set_entry(3, (unsigned int)isr3, ds, common_flags);
  idt_set_entry(4, (unsigned int)isr4, ds, common_flags);
  idt_set_entry(5, (unsigned int)isr5, ds, common_flags);
  idt_set_entry(6, (unsigned int)isr6, ds, common_flags);
  idt_set_entry(7, (unsigned int)isr7, ds, common_flags);
  idt_set_entry(8, (unsigned int)isr8, ds, common_flags);
  idt_set_entry(9, (unsigned int)isr9, ds, common_flags);
  idt_set_entry(10, (unsigned int)isr10, ds, common_flags);
  idt_set_entry(11, (unsigned int)isr11, ds, common_flags);
  idt_set_entry(12, (unsigned int)isr12, ds, common_flags);
  idt_set_entry(13, (unsigned int)isr13, ds, common_flags);
  idt_set_entry(14, (unsigned int)isr14, ds, common_flags);
  idt_set_entry(15, (unsigned int)isr15, ds, common_flags);
  idt_set_entry(16, (unsigned int)isr16, ds, common_flags);
  idt_set_entry(17, (unsigned int)isr17, ds, common_flags);
  idt_set_entry(18, (unsigned int)isr18, ds, common_flags);
  idt_set_entry(19, (unsigned int)isr19, ds, common_flags);
  idt_set_entry(20, (unsigned int)isr20, ds, common_flags);
  idt_set_entry(21, (unsigned int)isr21, ds, common_flags);
  idt_set_entry(22, (unsigned int)isr22, ds, common_flags);
  idt_set_entry(23, (unsigned int)isr23, ds, common_flags);
  idt_set_entry(24, (unsigned int)isr24, ds, common_flags);
  idt_set_entry(25, (unsigned int)isr25, ds, common_flags);
  idt_set_entry(26, (unsigned int)isr26, ds, common_flags);
  idt_set_entry(27, (unsigned int)isr27, ds, common_flags);
  idt_set_entry(28, (unsigned int)isr28, ds, common_flags);
  idt_set_entry(29, (unsigned int)isr29, ds, common_flags);
  idt_set_entry(30, (unsigned int)isr30, ds, common_flags);
  idt_set_entry(31, (unsigned int)isr31, ds, common_flags);

  exception_messages[0] = "Division By Zero";
  exception_messages[1] = "Debug";
  exception_messages[2] = "Non Maskable Interrupt";
  exception_messages[3] = "Breakpoint";
  exception_messages[4] = "Overflow";
  exception_messages[5] = "BOUND Range Exceeded";
  exception_messages[6] = "Invalid Opcode";
  exception_messages[7] = "Device Not Available";
  exception_messages[8] = "Double Fault";
  exception_messages[9] = "Coprocessor Segment Overrun";
  exception_messages[10] = "Invalid TSS";
  exception_messages[11] = "Segment Not Present";
  exception_messages[12] = "Stack-Segment Fault";
  exception_messages[13] = "General Protection Fault";
  exception_messages[14] = "Page Fault";
  exception_messages[15] = "Reserved";
  exception_messages[16] = "x87 FPU Floating-Point Error";
  exception_messages[17] = "Alignment Check";
  exception_messages[18] = "Machine Check";
  exception_messages[19] = "SIMD Floating-Point Exception";
  exception_messages[20] = "Virtualization Exception";
  exception_messages[21] = "Control Protection Exception";
  exception_messages[22] = "Reserved";
  exception_messages[23] = "Reserved";
  exception_messages[24] = "Reserved";
  exception_messages[25] = "Reserved";
  exception_messages[26] = "Reserved";
  exception_messages[27] = "Reserved";
  exception_messages[28] = "Hypervisor Injection Exception";
  exception_messages[29] = "VMM Communication Exception";
  exception_messages[30] = "Security Exception";
  exception_messages[31] = "Reserved";
}

/*
 
Generic fault handler called by all exception based ISRs.
For now, display a message and halt the CPU.

*/
void isr_fault_handler(struct regs *r) {
  if (r->int_no < 32) {
    print("System Halted!\n");

    print("Exception: ");
    print(exception_messages[r->int_no]);
    print("\nError Code: ");
    print_hex(r->err_code);
    print("\nIP: ");
    print_hex(r->eip);
    print("\nData Segment: ");
    print_hex(r->ds);
    print("\nCode Segment: ");
    print_hex(r->cs);

    for (;;) {
    }
  }
}
