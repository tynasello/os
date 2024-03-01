#include "../drivers/screen.h"
#include "include/system.h"

void _start() {
  print("\n");
  print("Entered into 32-bit Protected Mode.\n");
  print("Kernel loaded. Execution started at _start.\n");

  idt_init();
  print("IDT initialized and loaded.\n");
  isrs_init();
  irqs_init();
  timer_install();
  kb_install();
  print("ISRs initialized and loaded into IDT.\n");

  __asm__ __volatile__("sti"); // Re-enable interrups after the IDT and
                               // interrupt handlers have been initialized

  // __asm__ volatile("mov $0, %eax\n\t" // Cause divide-by-zero error
  //                  "mov $0, %edx\n\t"
  //                  "div %edx");
}
