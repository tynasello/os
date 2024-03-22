#include "include/idt.h"
#include "include/irq.h"
#include "include/isr.h"
#include "include/kb.h"
#include "include/paging.h"
#include "include/pmm.h"
#include "include/screen.h"
#include "include/timer.h"

void kmain() {
  print("\n");
  print("Entered into 32-bit Protected Mode.\n");
  print("Kernel loaded. Execution started at kmain.\n");

  idt_init();
  print("IDT initialized and loaded.\n");
  isrs_init();
  irqs_init();
  timer_install();
  kb_install();
  print("ISRs initialized and loaded into IDT.\n");

  __asm__ __volatile__("sti"); // Re-enable interrups after the IDT and
                               // interrupt handlers have been initialized
  
  print("End of kernel code located at: ");
  print_endkernel();
  print("\n");

  // int* i = (int*)0x3ff003;
  // *i = 4;
  // print_int(*i);
  // print("\n");

  pmm_init();
  page_init();
  print("Paging enabled.\n");

  // print_int(*i);

  // __asm__ volatile("mov $0, %eax\n\t" // Cause divide-by-zero error
  //                  "mov $0, %edx\n\t"
  //                  "div %edx");
}
