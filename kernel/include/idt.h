#ifndef __IDT_H
#define __IDT_H

void idt_init();
void idt_set_entry(unsigned char idt_num, unsigned int offset,
                   unsigned short seg_sel, unsigned char flags);

// Defines what the stack looks like after calling an ISR (a pointer to this
// struct is passed to the common fault handler)
struct regs {
  unsigned int gs, fs, es, ds;
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by 'pusha'
  unsigned int int_no, err_code; // Pushed interrupt number and error code
  unsigned int eip, cs, eflags, useresp,
      ss; // Automatically pushed by the processor when an interrupt is raised
};

#endif
