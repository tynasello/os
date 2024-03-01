#include "include/system.h"

// An entry in the IDT table
struct idt_entry {
  unsigned short
      offset_low; // The offset is the address of the ISR entrypoint that the
                  // processor will call when this interrupt is raised
  unsigned short seg_sel; // The segment that the ISR is located in. Must point
                          // to a valid code segment defined in GDT.
  unsigned char reserved; // Reserved for processor
  unsigned char
      flags; // Gate type (bits 0-3, 0xE represents a 32-bit interrupt gate),
             // bit 4 is always 0. DPL (bits 5-6, defines highest CPU privilege
             // level (ring) which is aloud to access this interrupt via 'int'
             // opcode). Present (bit 7, set to 1 if descriptor is valid).
  unsigned short offset_high;
} __attribute__((packed));

// The location of the IDT is stored in the IDTR (IDT register). This is loaded
// using the 'lidt' assembly instruction, whose argument is a pointer to an IDT
// Descriptor structure defined below:
struct idt_ptr {
  unsigned short size; // One less than size of IDT in bytes
  unsigned int offset; // Address of the IDT
} __attribute__((packed));

struct idt_entry
    idt[256]; // There are 256 interrupt vectors (0..255), so the
              // IDT should have 256 entries. If any undefined IDT entry or any
              // entry with a presence bit equal to 0 is hit, an 'Unhandled
              // Interrupt' exception will be generated.
struct idt_ptr idtp;

extern void idt_load();

void idt_set_entry(unsigned char idt_num, unsigned int offset,
                   unsigned short seg_sel, unsigned char flags) {
  struct idt_entry entry;

  entry.offset_low = offset;
  entry.offset_high = offset >> 16;
  entry.seg_sel = seg_sel;
  entry.reserved = 0x0;
  entry.flags = flags;

  idt[idt_num] = entry;
}

void idt_init() {
  // Initialize IDT pointer
  idtp.size = (sizeof(struct idt_entry) * 256) - 1;
  idtp.offset = (unsigned int)&idt;

  mem_set((unsigned char *)&idt, 0,
          sizeof(struct idt_entry) * 256); // Initialize IDT with zeros

  idt_load();
}
