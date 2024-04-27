#ifndef __IDT_H
#define __IDT_H

void idt_init();
void idt_set_entry(unsigned char idt_num, unsigned int offset,
                   unsigned short seg_sel, unsigned char flags);

/* Holds CPU register values at time of interrupt */
typedef struct {
  unsigned int gs, fs, es, ds;                          // Fault wrapper saves processor state
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  // ^
  unsigned int int_no, err_code;                        // Pushed interrupt number and error code
  unsigned int eip, cs, eflags, useresp, ss;            // Processor automatically pushes these registers when an interrupt is raised
} CpuContext;

#endif
