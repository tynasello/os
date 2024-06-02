#include "include/idt.h"
#include "include/screen.h"

/*
The frist 32 ISR defined in kernel_entry.asm.
Interrupt numbers 0-31 are reserved by Intel, and are designed to service
exceptions.
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

/* A Mapping from interrupt number to exception message */
char *exception_messages[32];

/*
 Initialize exception based ISRs:
Set entries 0-32 in the IDT to exception based ISRs.
*/
void isrs_init() {
  idt_set_gate(0, (uintptr_t)isr0);
  idt_set_gate(1, (uintptr_t)isr1);
  idt_set_gate(2, (uintptr_t)isr2);
  idt_set_gate(3, (uintptr_t)isr3);
  idt_set_gate(4, (uintptr_t)isr4);
  idt_set_gate(5, (uintptr_t)isr5);
  idt_set_gate(6, (uintptr_t)isr6);
  idt_set_gate(7, (uintptr_t)isr7);
  idt_set_gate(8, (uintptr_t)isr8);
  idt_set_gate(9, (uintptr_t)isr9);
  idt_set_gate(10, (uintptr_t)isr10);
  idt_set_gate(11, (uintptr_t)isr11);
  idt_set_gate(12, (uintptr_t)isr12);
  idt_set_gate(13, (uintptr_t)isr13);
  idt_set_gate(14, (uintptr_t)isr14);
  idt_set_gate(15, (uintptr_t)isr15);
  idt_set_gate(16, (uintptr_t)isr16);
  idt_set_gate(17, (uintptr_t)isr17);
  idt_set_gate(18, (uintptr_t)isr18);
  idt_set_gate(19, (uintptr_t)isr19);
  idt_set_gate(20, (uintptr_t)isr20);
  idt_set_gate(21, (uintptr_t)isr21);
  idt_set_gate(22, (uintptr_t)isr22);
  idt_set_gate(23, (uintptr_t)isr23);
  idt_set_gate(24, (uintptr_t)isr24);
  idt_set_gate(25, (uintptr_t)isr25);
  idt_set_gate(26, (uintptr_t)isr26);
  idt_set_gate(27, (uintptr_t)isr27);
  idt_set_gate(28, (uintptr_t)isr28);
  idt_set_gate(29, (uintptr_t)isr29);
  idt_set_gate(30, (uintptr_t)isr30);
  idt_set_gate(31, (uintptr_t)isr31);

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

/* An array of function pointers mapping ISR numbers to handler functions */
void *isr_handlers[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void isr_install_handler(int isr_no, void (*handler)(CpuContext *context)) {
  if (isr_no >= 0 && isr_no <= 31) {
    isr_handlers[isr_no] = handler;
  }
}

/*
Generic fault handler called by all exception based ISRs.
For now, display a message and halt the CPU.
*/
void isr_fault_handler(CpuContext *context) {
  if (context->int_no < 32) {
    print("\nSystem Halted!\n");

    print("Exception: ");
    print(exception_messages[context->int_no]);
    print("\n");

    print("Error Code: ");
    print_hex(context->err_code);
    print("\n");

    print("IP: ");
    print_hex(context->eip);
    print("\n");

    void (*handler)(CpuContext *context); // Blank function pointer
    handler = isr_handlers[context->int_no];
    if (handler) {
      handler(context);
    }

    while (1) {
    }
  }
}
