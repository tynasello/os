#include "include/idt.h"
#include "include/irq.h"
#include "include/isr.h"
#include "include/kb.h"
#include "include/vmm.h"
#include "include/pmm.h"
#include "include/screen.h"
#include "include/timer.h"

void kmain() {
  print("\n");
  print("Entered into 32-bit Protected Mode.\n");
  print("Kernel loaded. Execution started at kmain.\n");

  unsigned int stack_pointer;
  asm("mov %%esp, %0" : "=r" (stack_pointer)); 
  print("Stack locatated at: ");
  print_hex(stack_pointer);
  print("\n");

  idt_init();
  print("IDT initialized and loaded.\n");
  isrs_init();
  irqs_init();
  timer_install();
  kb_install();
  print("ISRs initialized and loaded into IDT.\n");

  __asm__ __volatile__("sti"); // Re-enable interrups after the IDT and interrupt handlers have been initialized
  
  print("End of kernel code located at: ");
  print_endkernel();
  print("\n");

  pmm_init();
  vmm_init();
  print("Paging enabled.\n");

  /* ------------- */

  // int* a = (int*) kmalloc(4097);
  // int* b = (int*) kmalloc(8);
  // int* c = (int*) kmalloc(4);
  // int* d = (int*) kmalloc(8);
  // int* e = (int*) kmalloc(4);
  // print_hex((uintptr_t)a);
  // print(", ");
  // print_hex((uintptr_t)b);
  // print(", ");
  // print_hex((uintptr_t)c);
  // print(", ");
  // print_hex((uintptr_t)d);
  // print(", ");
  // print_hex((uintptr_t)e);
  // print(", ");

  // __asm__ volatile("mov $0, %eax\n\t" // Cause divide-by-zero error
  //                  "mov $0, %edx\n\t"
  //                  "div %edx");
}
