#include "include/idt.h"
#include "include/memory.h"
#include "include/screen.h"
#include "stdint.h"
#include <stdint.h>

typedef struct {
  uint16_t offset_low; // The address of the ISR that the processor will call
                       // when this interrupt is raised
  uint16_t seg_sel; // The segment the ISR is located in. Must point to a valid
                    // code segment defined in GDT.
  uint8_t reserved; // Reserved for processor
  /* Gate type (bits 0-3, 0xE represents a 32-bit interrupt gate).
  Bit 4 is always 0.
  DPL (bits 5-6, defines highest CPU privilege level (ring) which is aloud to
  access this interrupt via 'int' opcode). Present (bit 7, set to 1 if
  descriptor is valid). */
  uint8_t flags;
  uint16_t offset_high;
} __attribute__((packed)) InterruptGateDescriptor;

typedef struct {
  uint16_t limit;
  uintptr_t base_addr;
} __attribute__((packed)) IdtDescriptor;

InterruptGateDescriptor idt[256];
IdtDescriptor idt_descriptor;

void idt_set_gate(uint8_t int_vec_num, uint32_t isr) {
  InterruptGateDescriptor entry;
  entry.offset_low = isr;
  entry.offset_high = isr >> 16;
  entry.seg_sel = 0x8; // CS
  entry.reserved = 0x0;
  entry.flags = 0x8E; // 10001110
  idt[int_vec_num] = entry;
}

/* Initialize IDT pointer, and load IDT initialized with zeroes (256
 * descriptors) */
void idt_init() {
  idt_descriptor.limit = (sizeof(InterruptGateDescriptor) * 256) - 1;
  idt_descriptor.base_addr = (uintptr_t)&idt;
  mem_set((unsigned char *)&idt, 0, sizeof(InterruptGateDescriptor) * 256);
  __asm__ __volatile__("lidt (%0)" : : "r"((uintptr_t)&idt_descriptor));
}

void print_cpu_context(CpuContext *context) {
  print("edi: ");
  print_hex(context->edi);
  print(", esi: ");
  print_hex(context->esi);
  print(", ebp: ");
  print_hex(context->ebp);
  print(", esp: ");
  print_hex(context->esp);
  print(", ebx: ");
  print_hex(context->ebx);
  print(", edx: ");
  print_hex(context->edx);
  print(", ecx: ");
  print_hex(context->ecx);
  print(", eax: ");
  print_hex(context->eax);
  print(", ");
  print("int_no: ");
  print_hex(context->int_no);
  print(", err_code: ");
  print_hex(context->err_code);
  print(", ");
  print("eip: ");
  print_hex(context->eip);
  print(", cs: ");
  print_hex(context->cs);
  print(", eflags: ");
  print_hex(context->eflags);
}
