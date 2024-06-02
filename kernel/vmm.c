/*

A virtual address space manager and allocator

Paging
- Paging provides processes with a larger address space, memory protection,
isolation, etc.
- Using one page directory and 4KiB pages, giving linear 4GiB (4KiB * 1024 *
1024) virtual address space from 0x0 - 0xFFFFFFFF

VMM
- Serves as an abstraction on top of the physcial memory manager and paging
- Currently, the VMM manages manages virtual memory (heaps) and page tables
- The VMM is capable of allocating page-sized memory through use of the PMM

Heap
- Facilities for dynamic allocation of byte-sized memory
- There is one kernel heap, while every process has its own user-space heap
- Currently using a doubly-linked list strategy with splitting and merging

*/

#include "include/vmm.h"
#include "include/idt.h"
#include "include/isr.h"
#include "include/memory.h"
#include "include/pmm.h"
#include "include/screen.h"
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define PD_RECURSIVE_I (NO_PDE - 1)

#define VA_PDI_START 22
#define VA_PTI_START 12

typedef enum {
  PT_PRESENT = (1 << 0),
  PT_WRITE = (1 << 1),
  PT_USER = (1 << 2),
} PtFlag;

typedef enum {
  VM_USED = (1 << 0),
  VM_MMIO = (1 << 3),
} VmFlag;

#define NO_FLAG_MASK 0xFFFF000

#define K_CODE_START 0xC0000000
#define K_CODE_END 0xD0000000

#define K_HEAP_START 0xD0000000
#define K_HEAP_END 0xE0000000

#define K_PAGE_START 0xF0000000
#define K_PAGE_END 0xFFBFFFFF

/*

Paging

*/

/*
x86 virtual address format:
- bits 22-31: page directory index
- bits 12-21: page table index
- bits 0-11: offset within the page
*/

void load_pd(uintptr_t pd_pa) {
  __asm__ __volatile__("mov %0, %%cr3" : : "r"(pd_pa) : "memory");
}

/*
The CPU uses the MMU to automatically walk the page tables and cache the
resulting translations in the translation lookaside buffer (TLB). This
buffer is not updated transparently and needs to be flushed manually on
page directory or table changes.

The INVLPG instruction instructs the processor to invalidate only the
region of the TLB associated with one page.
*/
void flush_tlb() {
  asm volatile("movl %%cr3, %%eax\n\t"
               "movl %%eax, %%cr3\n\t"
               :
               :
               : "eax", "memory");
}

/*
When the CPU raises a page-not-present exception, the CR2 register is populated
with the virtual address that caused the exception.

Format of CPU-pushed error code:
- bit 0: 1 if protection fault, 0 if non-present page entry
- bit 1: 1 if caused by read, 0 if caused by write
- bit 2: 1 if caused by user process, 0 if caused by supervising process
- bit 3: indicates whether a reserved bit was set in some page-structure entry
- bit 4: is the instruction/data flag (1 if instruction fetch, 0 if data access)
- bit 5: indicates a protection-key violation
- bit 6: indicates a shadow-stack access fault
- bit 15: indicates an SGX violaton
*/
void page_fault_handler(CpuContext *context) {
  uint32_t cr2_value;
  asm volatile("mov %%cr2, %0" : "=r"(cr2_value));

  // int present = r->err_code & 0x1;
  // int rw = (r->err_code >> 1) & 0x1;
  // int us = (r->err_code >> 2) & 0x1;

  print("Accessed virtual address: ");
  print_hex(cr2_value);
  print("\n");
}

/* Page Table Methods */

static uint32_t va_to_pde_i(uintptr_t va) { return va >> VA_PDI_START & 0x3FF; }

static uint32_t va_to_pte_i(uintptr_t va) { return va >> VA_PTI_START & 0x3FF; }

static uint32_t is_pde_empty(uintptr_t va) {
  uint32_t pde_i = va_to_pde_i(va);
  return ((uintptr_t)(((Pd *)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START |
                              PD_RECURSIVE_I << VA_PTI_START))
                          ->pts[pde_i]) &
          PT_PRESENT) == 0;
}

static uint32_t is_pte_empty(uintptr_t va) {
  uint32_t pde_i = va_to_pde_i(va);
  if (is_pde_empty(va)) {
    return 1;
  }
  uint32_t pte_i = va_to_pte_i(va);
  return ((uintptr_t)(((Pd *)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START |
                              pde_i << VA_PTI_START))
                          ->pts[pte_i]) &
          PT_PRESENT) == 0;
}

static void create_pde(uintptr_t va) {
  if (!is_pde_empty(va)) {
    return;
  }
  uint32_t pde_i = va_to_pde_i(va);
  (((Pd *)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | PD_RECURSIVE_I
                                                           << VA_PTI_START)))
      ->pts[pde_i] = (Pt *)((uintptr_t)alloc_frame() | PT_PRESENT | PT_WRITE);
}

static void create_pte(uintptr_t va, uintptr_t frame) {
  if (!is_pte_empty(va)) {
    return;
  }
  uint32_t pde_i = va_to_pde_i(va);
  uint32_t pte_i = va_to_pte_i(va);
  if (is_pde_empty(va)) {
    create_pde(va);
  }
  (((Pt *)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | pde_i << VA_PTI_START)))
      ->frames[pte_i] = (uintptr_t)(frame | PT_PRESENT | PT_WRITE);
}

uintptr_t kmalloc(uint32_t no_bytes);

/* The kernel PD wil be the head */
ProcessPd *process_pds = NULL;

/*
Create an empty page directory for another process.
Returns a pointer to a struct representing a processes PD.
*/
ProcessPd *create_process_pd() {
  ProcessPd *curr = process_pds;
  while (curr->next != NULL) {
    curr = curr->next;
  }

  ProcessPd *process_pd = (ProcessPd *)kmalloc(sizeof(ProcessPd));
  process_pd->pd_va =
      curr == process_pds ? (Pd *)K_PAGE_START : curr->pd_va + 1;
  process_pd->pd_pa = alloc_frame();
  process_pd->next = NULL;
  curr->next = process_pd;
  create_pte((uintptr_t)process_pd->pd_va, process_pd->pd_pa);

  /* Initialize PD */
  for (int i = 0; i < NO_PDE; i++) {
    process_pd->pd_va->pts[i] = (Pt *)0x0;
  }

  /* Recursive Entry */
  process_pd->pd_va->pts[PD_RECURSIVE_I] =
      (Pt *)(process_pd->pd_pa | PT_WRITE | PT_PRESENT);

  /* Map Kernel */
  int kernel_pde = va_to_pde_i(K_CODE_START);
  process_pd->pd_va->pts[kernel_pde] = (Pt *)(0x7F000 | PT_PRESENT);

  for (int i = kernel_pde; i < 1023; i++) {
    if (process_pd->pd_va->pts[i] != process_pds->pd_va->pts[i]) {
      process_pd->pd_va->pts[i] =
          (Pt *)(((uintptr_t)process_pds->pd_va->pts[i] & NO_FLAG_MASK) |
                 PT_PRESENT);
    }
  }

  return process_pd;
}

void delete_process_pd(ProcessPd *process_pd) {
  if (process_pd == process_pds) {
    return;
  }

  int kernel_pde = va_to_pde_i(K_CODE_START);

  for (int i = kernel_pde; i < 1023; i++) {
    free_frame((uintptr_t)process_pds->pd_va->pts[i]);
    process_pd->pd_va->pts[i] = NULL;
  }
  free_frame((uintptr_t)process_pd->pd_va->pts[kernel_pde]);
  process_pd->pd_va->pts[kernel_pde] = NULL;

  for (int i = 0; i < NO_PDE; i++) {
    free_frame((uintptr_t)process_pd->pd_va->pts[i]);
    process_pd->pd_va->pts[i] = NULL;
  }

  free_frame((uintptr_t)process_pd->pd_va->pts[PD_RECURSIVE_I]);
  process_pd->pd_va->pts[PD_RECURSIVE_I] = NULL;

  ProcessPd *curr = process_pds;
  while (curr->next != process_pd) {
    curr = curr->next;
  }
  curr->next = process_pd->next;

  free_frame(process_pd->pd_pa);
  process_pd->next = NULL;
  process_pd->pd_pa = 0;
  process_pd->pd_va = NULL;
  kfree(process_pd);
  process_pd = NULL;
}

/*

Page directory/page tables are initialized and paging is enabled in boot sector.

- The higher-half kernel code is linked at VA 0xC0000500 and is mapped to PA
0x500.
- To access page table frames, we need to map virtual addresses to them. We do
  this using a recursive mapping in the last entry of the page directory.
- Created page directories will be stored at 0xF0000000.
- cr3 is the page directory base register. On each memory access, the CPU reads
  the table pointer from this register and looks up the mapped frame for a given
  virtual address.
- The OS can change cr3, and in doing so, different page directories can be used
  to give separate processes/applications different address spaces.
- MMU issues a page fault if it encounters a non-present page when walking
tables.

*/

void vm_init() {
  /* Install page fault handler */
  isr_install_handler(14, page_fault_handler);

  /* Remove temp PDE created during boot */
  (((Pd *)(PD_RECURSIVE_I << VA_PDI_START | PD_RECURSIVE_I << VA_PTI_START)))
      ->pts[0] = 0x0;
  flush_tlb();

  /* Record kernel PD */
  process_pds = (ProcessPd *)kmalloc(sizeof(ProcessPd));
  process_pds->pd_va = (Pd *)(K_CODE_START + 0x7E000);
  process_pds->pd_pa = (uintptr_t)0x7E000;
  ;
  process_pds->next = NULL;
}

/*

Heap

*/

/* A variable-sized section of the virtual address space */
typedef struct VmNode {
  uint32_t no_bytes;
  uint8_t flags; // bit 0 represents free or used
  struct VmNode *next;
  struct VmNode *prev;
} VmNode;

/* A representaion of a virtual memory range (can be heap). Each process will
 * have its own VM range for its heap. */
typedef struct {
  VmNode *head;
  VmNode *tail;
} VmRange;

void print_heap(VmRange *heap) {
  print("Heap located at VA: ");
  print_hex((uintptr_t)heap);
  print("\n");

  VmNode *curr = heap->head;

  while (curr != NULL) {
    print_hex((uintptr_t)curr);
    print(" <- (");
    print_int((uintptr_t)curr->no_bytes);
    print("B, ");
    print_int(curr->flags);
    print(") -> ");
    curr = curr->next;
  }
  print_hex((uintptr_t)curr);
  print("\n");
}

VmRange *vmm_init(uintptr_t vm_range_start);
uint8_t vmm_alloc(VmRange *vm_range, uint8_t flags, uintptr_t arg);

/*

Kernel Space

*/

VmRange *k_heap = NULL;

/*
Dynamically allocate aribtrarily-sized regions of memory
*/
uintptr_t kmalloc(uint32_t no_bytes) {
  /* Heap initialization */
  if (k_heap == NULL) {
    k_heap = vmm_init(K_HEAP_START);
    if (k_heap == 0) {
      return 0;
    }
  }

  VmNode *curr = k_heap->head;

  while (no_bytes > curr->no_bytes || curr->flags & VM_USED) {
    /* Expand heap if out of space */
    if (curr == k_heap->tail) {
      /* Upper bound on kernel heap */
      if ((uintptr_t)curr > K_HEAP_END - PAGE_SIZE) {
        return 0;
      }
      if (vmm_alloc(k_heap, 0, 0)) {
        return 0;
      }
      curr = k_heap->tail;
    } else {
      curr = curr->next;
    }
  }

  /* Split heap nodes if possible (only if unused node in split has atleast 16
   * free bytes) */
  if (curr->no_bytes - no_bytes > sizeof(VmNode) + 0x10) {
    VmNode *right = (VmNode *)((uintptr_t)curr + sizeof(VmNode) + no_bytes);
    right->no_bytes = curr->no_bytes - no_bytes - sizeof(VmNode);
    right->flags = 0x0;
    right->next = curr->next;
    right->prev = curr;
    curr->no_bytes = no_bytes;
    curr->next = right;
    if (curr == k_heap->tail) {
      k_heap->tail = right;
    }

    /* Merge right with respective right if possible */
    if (right->next != NULL && !(right->next->flags & VM_USED)) {
      VmNode *tmp = right->next;
      right->next = tmp->next;
      right->no_bytes += (tmp->no_bytes + sizeof(VmNode));
      if (tmp->next != NULL) {
        tmp->next->prev = right;
      }
      if (tmp == k_heap->tail) {
        k_heap->tail = right;
      }
      mem_set((uint8_t *)tmp, 0x0, sizeof(VmNode) + tmp->no_bytes);
    }
  }

  curr->flags = VM_USED;

  return (uintptr_t)curr + sizeof(VmNode);
}

/*
Mark a given heap node as free for use, and merge it with its neighbours if they
are also free. No real physical memory is freed by this function, virtual memory
ranges are essentially just marked for reuse. Return 1 if VA cannot be freed
*/
int kfree(void *va) {
  VmNode *va_node = va - sizeof(VmNode);

  /* Ensure VA node is in heap range */
  if (va_node < (VmNode *)K_HEAP_START ||
      (uintptr_t)va_node + sizeof(VmNode) > K_HEAP_END ||
      (uintptr_t)va_node + sizeof(VmNode) + va_node->no_bytes > K_HEAP_END) {
    return 1;
  }

  /* Free node */
  mem_set((uint8_t *)((uintptr_t)va_node) + sizeof(VmNode), 0x0,
          va_node->no_bytes);
  va_node->flags = 0x0;

  /* Merge with left if possible */
  if (k_heap->head->next != k_heap->tail && va_node->prev != NULL &&
      (va_node->prev->flags & VM_USED) == 0) {
    VmNode *left = va_node->prev;
    left->no_bytes += (sizeof(VmNode) + va_node->no_bytes);
    left->next = va_node->next;
    if (va_node->next != NULL) {
      va_node->next->prev = left;
    }
    if (k_heap->tail == va_node) {
      k_heap->tail = left;
    }
    mem_set((uint8_t *)va_node, 0x0, sizeof(VmNode) + va_node->no_bytes);
  }

  /* Merge with right if possible */
  if (k_heap->head->next != k_heap->tail && va_node->next != NULL &&
      (va_node->next->flags & VM_USED) == 0) {
    VmNode *right = va_node->next;
    va_node->no_bytes += (sizeof(VmNode) + right->no_bytes);
    va_node->next = right->next;
    if (right->next != NULL) {
      right->next->prev = va_node;
    }
    if (k_heap->tail == right) {
      k_heap->tail = va_node;
    }
    mem_set((uint8_t *)right, 0x0, sizeof(VmNode) + right->no_bytes);
  }

  return 0;
};

/*

User space

*/

uintptr_t malloc(uint32_t no_bytes) { return 0; }

void free() {};

/*

VMM

*/

/*
Initialize a VM region with ~8KiB of memory.
Return pointer to head of VM range, and 0 if not possible.
*/
VmRange *vmm_init(uintptr_t vm_range_start) {
  uintptr_t allocated_frame_head = alloc_frame();
  uintptr_t allocated_frame_tail = alloc_frame();
  if (!allocated_frame_head || !allocated_frame_tail) {
    return 0;
  }

  VmRange *vm_range = (VmRange *)vm_range_start;

  create_pte((uintptr_t)vm_range, allocated_frame_head);
  vm_range->head =
      (VmNode *)(vm_range_start + sizeof(VmRange *) + sizeof(VmRange));

  /* Head */

  uint32_t available_bytes =
      PAGE_SIZE - sizeof(VmRange *) - sizeof(VmRange) - sizeof(VmNode);
  VmNode *new_vm_range = vm_range->head;
  create_pte((uintptr_t)new_vm_range, allocated_frame_head);

  new_vm_range->prev = NULL;
  new_vm_range->next = NULL;
  new_vm_range->no_bytes = available_bytes;
  new_vm_range->flags = 0x0;

  /* Tail */

  available_bytes = PAGE_SIZE - sizeof(VmNode);
  new_vm_range = (VmNode *)((uintptr_t)(vm_range->head) + sizeof(VmNode) +
                            vm_range->head->no_bytes);
  create_pte((uintptr_t)new_vm_range, allocated_frame_tail);

  new_vm_range->prev = vm_range->head;
  new_vm_range->next = NULL;
  new_vm_range->no_bytes = available_bytes;
  new_vm_range->flags = 0x0;

  vm_range->head->next = new_vm_range;
  vm_range->tail = new_vm_range;

  return vm_range;
}

/*
Function to expand a given VM range. Return 1 if not possible.
Current strategy is to immediately back new nodes with physcial memory.
Can adjust this to allocate memory on demand using page fault handler.
- Here, malloc would call a morecore function to expand the heap, and the
  page fault handler would call vmm_alloc to allocate and map physical memory.
*/
uint8_t vmm_alloc(VmRange *vm_range, uint8_t flags, uintptr_t arg) {
  uintptr_t allocated_frame = 0x0;
  if (flags & VM_MMIO) {
    allocated_frame = arg;
  } else {
    allocated_frame = alloc_frame();
    if (!allocated_frame) {
      return 1;
    }
  }

  uint32_t available_bytes = PAGE_SIZE - sizeof(VmNode);
  VmNode *new_vm_range = (VmNode *)((uintptr_t)(vm_range->tail) +
                                    sizeof(VmNode) + vm_range->tail->no_bytes);
  create_pte((uintptr_t)new_vm_range, allocated_frame);
  create_pte((uintptr_t)new_vm_range + PAGE_SIZE - 1, allocated_frame);

  new_vm_range->prev = vm_range->tail;
  new_vm_range->next = NULL;
  new_vm_range->no_bytes = available_bytes;
  new_vm_range->flags = flags;

  vm_range->tail->next = new_vm_range;
  vm_range->tail = new_vm_range;

  /* Merge nodes (only if new node follows another free node) */
  if (vm_range->head->next != vm_range->tail &&
      (vm_range->tail->prev->flags & VM_USED) == 0) {
    VmNode *left = vm_range->tail->prev;
    left->no_bytes += (sizeof(VmNode) + vm_range->tail->no_bytes);
    left->next = NULL;
    mem_set((uint8_t *)vm_range->tail, 0x0,
            sizeof(VmNode) + vm_range->tail->no_bytes);
    vm_range->tail = left;
  }

  return 0;
}

/* Cleanup VMM that is no longer in use */
void vmm_destroy(VmRange *vm_range) {}
