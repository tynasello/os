#include "include/idt.h"
#include "include/irq.h"
#include "include/isr.h"
#include "include/kb.h"
#include "include/snake.h"
#include "include/system.h"
#include "include/vmm.h"
#include "include/pmm.h"
#include "include/screen.h"
#include "include/timer.h"

void kmain() {
  print("\nEntered into 32-bit Protected Mode.\nKernel loaded. Execution started at kmain.\n");
  
  pmm_init();
  vm_init();
  print("Paging enabled.\n");

  uint32_t stack_pointer;
  asm("mov %%esp, %0" : "=r" (stack_pointer)); 
  print("Stack locatated at VA: ");
  print_hex(stack_pointer);
  print("\n");

  extern uintptr_t endkernel;
  print("End of kernel locatated at VA: ");
  print_hex((uintptr_t)&endkernel);
  print("\n");

  idt_init();
  print("IDT initialized and loaded.\n");
  isrs_init();
  irqs_init();
  timer_install();
  kb_install();
  print("ISRs initialized and loaded into IDT.\n");

  __asm__ __volatile__("sti"); // Re-enable interrups after the IDT and interrupt handlers have been initialized

  snake_start();
}
