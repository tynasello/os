#ifndef __SYSTEM_H
#define __SYSTEM_H

void mem_cpy(const unsigned char *source, unsigned char *dest, int no_bytes);
void mem_set(unsigned char *dest, unsigned char val, int no_bytes);
void mem_set_word(unsigned short *dest, unsigned short val, int no_words);

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

void isrs_init();

void irqs_init();
void irq_install_handler(int irq_no, void (*handler)(struct regs *r));

void timer_install();
void timer_wait(int secs);

void kb_install();

#endif
