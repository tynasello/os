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
  print("\n");
  print("Entered into 32-bit Protected Mode.\n");
  print("Kernel loaded. Execution started at kmain.\n");

  uint32_t stack_pointer;
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

  pmm_init();
  vm_init();
  print("Paging enabled.\n");
  
  // snake_strt();
  
  /* ------------- */

  // int* a = (int*)kmalloc(4000);
  // *a = 9;
  // int* b = (int*)kmalloc(4000);
  // *b = 9;
  // int* c = (int*)kmalloc(4000);
  // *c = 9;
  // int* d = (int*)kmalloc(4000);
  // *d = 9;
  // kfree(a);
  // kfree(b);

  // for (int i = 0; i < 3; i++){
  //   int* a = (int*)kmalloc(4);
  //   *a = i;
  //   kfree(a);
  //   print_int((uintptr_t)*a);
  //   print_hex((uintptr_t)a);
  //   print(", ");
  // }

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

  // // Page fault
  // int*a = (int*) 0xA000000;
  // *a = 3;

  // // Divide by zero
  // __asm__ volatile("mov $0, %eax\n\t" // Cause divide-by-zero error
  //                  "mov $0, %edx\n\t"
  //                  "div %edx");
}
