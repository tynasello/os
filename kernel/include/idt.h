#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>

void idt_init();
void idt_set_gate(uint8_t int_vec_num, uint32_t isr);

/*
When an interrupt occurs, the custom ISR wrapper pushes the following:
- General-purpose registes: edi, esi , ebp, esp (value before pushing prev
regs, useless and ignored), ebx, edx, ecx, eax
- Segment registers: gs, fs, es, ds
- int_no, err_code (for exceptions, may be pushed automatically by processor)

The processor automatically pushes the following on a call to a
interrupt-handler :
- eip (instruction pointer)
- cs (code segment)
- eflags (processor flags)
and the following when crossing rings (like user to kernel)
- esp
- ss
*/
typedef struct {
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags;
} CpuContext;

void print_cpu_context(CpuContext *context);

#endif
