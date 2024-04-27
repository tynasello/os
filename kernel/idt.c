/*
--------------------- 

Interrupt Descriptor Table

--------------------- 
*/ 

#include "include/screen.h"
#include "include/system.h"
#include "stdint.h"
#include <stdint.h>

struct idt_entry {
  /* The address of the ISR that the processor will call when this interrupt is raised */
  uint16_t offset_low; 
  /* The segment the ISR is located in. Must point to a valid code segment defined in GDT. */
  uint16_t seg_sel; 
  uint8_t reserved; // Reserved for processor
  /*
  Gate type (bits 0-3, 0xE represents a 32-bit interrupt gate).
  Bit 4 is always 0. 
  DPL (bits 5-6, defines highest CPU privilege level (ring) which is aloud to access this interrupt via 'int' opcode). 
  Present (bit 7, set to 1 if descriptor is valid).
  */
  uint8_t flags; 
  uint16_t offset_high;
} __attribute__((packed));

/*
The location of the IDT is stored in the IDTR (IDT register). This is loaded
using the 'lidt' assembly instruction, whose argument is a pointer to an IDT
Descriptor structure defined below:
*/
struct idt_ptr {
  uint16_t size;            // One less than size of IDT in bytes
  uintptr_t offset;         // Address of the IDT
} __attribute__((packed));

struct idt_entry idt[256]; 
struct idt_ptr idtp;

void idt_set_entry(unsigned char idt_num, unsigned int offset, unsigned short seg_sel, unsigned char flags) {
  struct idt_entry entry;
  entry.offset_low = offset;
  entry.offset_high = offset >> 16;
  entry.seg_sel = seg_sel;
  entry.reserved = 0x0;
  entry.flags = flags;
  idt[idt_num] = entry;
}

/*
Initialize IDT pointer, and load IDT initialized with zeroes
*/
void idt_init() {
  idtp.size = (sizeof(struct idt_entry) * 256) - 1;
  idtp.offset = (uintptr_t)&idt;
  mem_set((unsigned char *)&idt, 0, sizeof(struct idt_entry) * 256); 
  __asm__ __volatile__("lidt (%0)" : : "r" ((uintptr_t)&idtp));
}

