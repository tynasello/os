#include "include/idt.h"
#include "include/irq.h"
#include "include/isr.h"
#include "include/kb.h"
#include "include/pmm.h"
#include "include/screen.h"
#include "include/test.h"
#include "include/timer.h"
#include "include/vmm.h"

void kmain() {
  print("\nEntered into 32-bit Protected Mode.\n");

  uint32_t stack_pointer;
  asm("mov %%esp, %0" : "=r"(stack_pointer));
  print("Stack locatated at VA: ");
  print_hex(stack_pointer);
  print("\n");

  extern uintptr_t endkernel;
  print("End of kernel locatated at VA: ");
  print_hex((uintptr_t)&endkernel);
  print("\n");

  idt_init();
  print("IDT initialized.\n");
  isrs_init();
  irqs_init();
  timer_install();
  kb_install();
  print("ISRs initialized.\n");

  pmm_init();
  vm_init();
  print("Physical and virtual memory managers initialized.\n");

  __asm__ __volatile__("sti"); // Re-enable interrups after the IDT and
                               // interrupt handlers have been initialized

  test_vm();

  while (1) {
  }
}
