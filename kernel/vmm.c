/*
--------------------- 

Virtual Memory Manager

A virtual address space manager and allocator

Paging
- Paging provides processes with a larger address space, memory protection, isolation, etc.
- Using one page directory and 4KiB pages, giving linear 4GiB (4KiB * 1024 * 1024) 
  virtual address space from 0x0 - 0xFFFFFFFF

VMM 
- Serves as an abstraction on top of the physcial memory manager and paging
- Currently, the VMM manages manages virtual memory (heaps) and page tables
- The VMM is capable of allocating page-sized memory through use of the PMM

Heap
- Facilities for dynamic allocation of byte-sized memory
- There is one kernel heap, while every process has its own user-space heap
- Currently using a doubly-linked list strategy with splitting and merging

--------------------- 
*/ 

#include "include/idt.h"
#include "include/pmm.h"
#include "include/screen.h"
#include "include/isr.h"
#include "include/system.h"
#include <stddef.h>
#include <stdint.h>

#define NO_PDE 1024
#define NO_PTE 1024
#define PAGE_SIZE 4096
#define PD_RECURSIVE_I (NO_PDE - 1)

#define VA_PDI_START 22
#define VA_PTI_START 12

typedef enum {
  PT_PRESENT = (1 << 0),
  PT_WRITE = (1 << 1),
  PT_USER = (1 << 2),
} PT_FLAG;

typedef enum {
  VM_USED = (1 << 0),
  VM_MMIO = (1 << 3),
} VM_FLAG;

#define K_HEAP_START 0xD0000000
#define K_HEAP_END 0xE0000000

#define HEAP_START 0xB0000000
#define HEAP_END 0xC0000000

#define K_PAGE_START 0xF0000000
#define K_PAGE_END 0xFFBFFFFF

/*

Paging

*/

/*
Page table entry format:
- bit 0: present: page is currently in memory.
- bit 1: write enabled: can be both read from and written to
- bit 2: user-accessible: if not set, only kernel mode code can access this page
- bit 3: write-through
- bit 4: cache-disable
- bit 5: accessed (set by CPU)
- bit 6: dirty: page has been written to (set by CPU)
- bit 7: page attribute table
- bit 8: global
- bit 9-11: free for use by OS
- bit 12-31: bits 12-31 of physical address of a page frame
*/
typedef struct {
  uintptr_t frames[NO_PTE]; // Each entry is a (physical) address to a page frame
} pt_t;

/*
Page directory entry format:
- bit 0: present: page is currently in memory.
- bit 1: write enabled: can be both read from and written to
- bit 2: user-accessible: if not set, only kernel mode code can access this page
- bit 3: write-through
- bit 4: cache-disable
- bit 5: accessed
- bit 7: page size
- bit 6, 8-11: free for use by OS
- bit 12-31: bits 12-31 of physcial address of a page table
*/
typedef struct {
  pt_t* pts[NO_PDE]; // Each entry is a (physical) address to a page table
} pd_t;

/* A linked list representing all the current page directories */
typedef struct process_pd_t{
  pd_t* pd_va;
  uintptr_t pd_pa;
  struct process_pd_t* next;
} process_pd_t;

/* The kernel PD wil be the head */
process_pd_t* process_pd_head = NULL;

/*
x86 virtual address format:
- bits 22-31: page directory index
- bits 12-21: page table index
- bits 0-11: offset within the page
*/
static uint32_t va_to_pde_i(uintptr_t va) {
  return va >> VA_PDI_START & 0x3FF;
}

static uint32_t va_to_pte_i(uintptr_t va) {
  return va >> VA_PTI_START & 0x3FF;
}

static uint32_t pde_empty(uintptr_t va) {
  uint32_t pde_i = va_to_pde_i(va);
  return ((uintptr_t)(((pd_t*)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | PD_RECURSIVE_I << VA_PTI_START))->pts[pde_i]) & PT_PRESENT) == 0;
}

static uint32_t pte_empty(uintptr_t va) {
  uint32_t pde_i = va_to_pde_i(va);
  if (pde_empty(va)) {
    return 1;
  }
  uint32_t pte_i = va_to_pte_i(va);
  return ((uintptr_t)(((pd_t*)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | pde_i << VA_PTI_START))->pts[pte_i]) & PT_PRESENT) == 0;
}

static void create_pde(uintptr_t va) {
  uint32_t pde_i = va_to_pde_i(va);
  (((pd_t*)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | PD_RECURSIVE_I << VA_PTI_START)))->pts[pde_i] = (pt_t*)((uintptr_t)alloc_frame() | PT_PRESENT | PT_WRITE);
}

static void create_pte(uintptr_t va, uintptr_t frame) {
  if (!pte_empty(va)) {
    return;
  }
  uint32_t pde_i = va_to_pde_i(va);
  uint32_t pte_i = va_to_pte_i(va);
  if (pde_empty(va)) {
    create_pde(va);
  }
  (((pt_t*)((uintptr_t)PD_RECURSIVE_I << VA_PDI_START | pde_i << VA_PTI_START)))->frames[pte_i] = (uintptr_t)(frame | PT_PRESENT | PT_WRITE);
}

uintptr_t kmalloc(uint32_t no_bytes);

/* 
Create an empty page directory for a user process.
Returns a pointer to a struct containing a processes' page directory.
*/
static process_pd_t* create_user_pd() {
  process_pd_t* curr = process_pd_head;
  while (curr->next != NULL){
    curr = curr->next;
  }

  process_pd_t* pd = (process_pd_t*)kmalloc(sizeof(process_pd_t));
  pd->pd_va = curr->pd_va + 1;
  pd->pd_pa = (uintptr_t)alloc_frame(); 
  pd->next = NULL;
  curr->next = pd;
  create_pte((uintptr_t)pd->pd_va, pd->pd_pa);

  /* Initialize PD */
  for (int i = 0; i < NO_PDE; i++) {
    pd->pd_va->pts[i] = (pt_t*)0x0;
  }
  pd->pd_va->pts[PD_RECURSIVE_I] = (pt_t *)((uintptr_t)pd->pd_pa | PT_USER | PT_WRITE | PT_PRESENT);

  return pd;
}

static void load_pd(uintptr_t pd_pa) {
  asm volatile("mov %0, %%cr3" :: "r"(pd_pa): "memory");
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
void page_fault_handler(struct regs *r){
  uint32_t cr2_value;
  asm volatile("mov %%cr2, %0" : "=r" (cr2_value));

  // int present = r->err_code & 0x1;
  // int rw = (r->err_code >> 1) & 0x1;
  // int us = (r->err_code >> 2) & 0x1;

  print("Accessed virtual page: ");
  print_hex(cr2_value);
  print("\n");
}

/*
Initialize page directory and page tables.
- Identity-map the real-mode address space (< 1MiB), this way kernel code, 
  stack, and video memory are still accessible using same addresses after 
  paging is enabled. 
- To access page table frames, we need to map virtual addresses to them. We do 
  this using a recursive mapping in the last entry of the page directory.
- Page directories are stored at 0xF0000000.

Load page directory.
- cr3 is the page directory base register. On each memory access, the CPU reads
  the table pointer from this register and looks up the mapped frame for a given 
  virtual address.
- The OS can change cr3, and in doing so, different page directories can be used 
  to give separate processes/applications different address spaces.
- MMU issues a page fault if it encounters a non-present page when walking tables.

Enable paging.
- Done through use of cr0 register.
*/

pd_t* kpd_pa = NULL;

void vm_init() {
  kpd_pa = (pd_t *)alloc_frame(); // PA of kernel page directory
  for (int i = 0; i < NO_PDE; i++) {
    kpd_pa->pts[i] = (pt_t*)0x0;
  }
  /* Recursively map the page directory. */
  kpd_pa->pts[PD_RECURSIVE_I] = (pt_t *)((uintptr_t)kpd_pa | PT_PRESENT);

  /* Identity-map real-mode address space */
  pt_t *kpt = (pt_t *) alloc_frame();
  kpd_pa->pts[0] = (pt_t*)((uintptr_t)kpt | PT_PRESENT);
  int i = 0;
  while (i * PAGE_SIZE <= 0xFF000) {
    kpt->frames[i] = i * PAGE_SIZE | PT_PRESENT;
    i++;
  }

  /* Install page fault handler */
  isr_install_handler(14, page_fault_handler);

  /* Load PD */
  load_pd((uintptr_t)kpd_pa);

  /* Enable paging */
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" :: "r"(cr0));

  process_pd_head = (process_pd_t*)kmalloc(sizeof(process_pd_t));
  process_pd_head->pd_va = (pd_t*)K_PAGE_START;
  process_pd_head->pd_pa = (uintptr_t)kpd_pa;
  process_pd_head->next = NULL;
  create_pte((uintptr_t)process_pd_head->pd_va, process_pd_head->pd_pa);
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
  asm volatile (
    "movl %%cr3, %%eax\n\t"
    "movl %%eax, %%cr3\n\t"
    :
    :
    : "eax", "memory"
  );
}

/*
 
Heap

*/

/* A variable-sized section of the virtual address space */
typedef struct vm_node_t{
  uint32_t no_bytes;
  uint8_t flags;            // bit 0 represents free or used
  struct vm_node_t* next;
  struct vm_node_t* prev;
} vm_node_t;

/* A representaion of a virtual memory range (can be heap). Each process will have its own VM range for its heap. */
typedef struct {
  vm_node_t* head;
  vm_node_t* tail;
} vm_range_t;

void print_heap(vm_range_t* heap){
  print("Heap located at VA: ");
  print_hex((uintptr_t)heap);
  print("\n");

  vm_node_t* curr = heap->head;

  while(curr != NULL){
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

vm_range_t* vmm_init(uintptr_t vm_range_start);
uint8_t vmm_alloc(vm_range_t* vm_range, uint8_t flags, uintptr_t arg);

/*

Kernel Space

*/

vm_range_t* k_heap = NULL;

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

  vm_node_t* curr = k_heap->head;
  
  while (no_bytes > curr->no_bytes || curr->flags & VM_USED) {
    /* Expand heap if out of space */
    if (curr == k_heap->tail) {
      /* Upper bound on kernel heap */
      if ((uintptr_t)curr > K_HEAP_END - PAGE_SIZE) {
        return 0;
      }
      if (vmm_alloc(k_heap, 0, 0)){
        return 0;
      }
      curr = k_heap->tail;
    } else {
      curr = curr->next;
    }
  }

  /* Split heap nodes if possible (only if unused node in split has atleast 16 free bytes) */
  if (curr->no_bytes - no_bytes > sizeof(vm_node_t) + 0x10) { 
    vm_node_t* right = (vm_node_t*)((uintptr_t)curr + sizeof(vm_node_t) + no_bytes);
    right->no_bytes = curr->no_bytes - no_bytes - sizeof(vm_node_t);
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
      vm_node_t* tmp = right->next;
      right->next = tmp->next;
      right->no_bytes += (tmp->no_bytes + sizeof(vm_node_t));
      if (tmp->next != NULL) {
        tmp->next->prev = right;
      }
      if (tmp == k_heap->tail) {
        k_heap->tail = right;
      }
      mem_set((uint8_t*)tmp, 0x0, sizeof(vm_node_t) + tmp->no_bytes);
    }
  }

  curr->flags = VM_USED;

  return (uintptr_t)curr + sizeof(vm_node_t);
}

/*
Mark a given heap node as free for use, and merge it with its neighbours if they are also free.
No real physical memory is freed by this function, virtual memory ranges are essentially just marked for reuse.
Return 1 if VA cannot be freed
*/
int kfree(void* va) {
  vm_node_t* va_node = va - sizeof(vm_node_t);

  /* Ensure VA node is in heap range */
  if (va_node < (vm_node_t*) K_HEAP_START || (uintptr_t)va_node + sizeof(vm_node_t) > K_HEAP_END || 
      (uintptr_t)va_node + sizeof(vm_node_t) + va_node->no_bytes > K_HEAP_END) {
    return 1;
  }

  /* Free node */
  mem_set((uint8_t*)((uintptr_t)va_node) + sizeof(vm_node_t), 0x0, va_node->no_bytes);
  va_node->flags = 0x0;

  /* Merge with left if possible */ 
  if (k_heap->head->next != k_heap->tail && va_node->prev != NULL && (va_node->prev->flags & VM_USED) == 0) {
    vm_node_t* left = va_node->prev;
    left->no_bytes += (sizeof(vm_node_t) + va_node->no_bytes);
    left->next = va_node->next;
    if (va_node->next != NULL) {
      va_node->next->prev = left;
    }
    if (k_heap->tail == va_node) {
      k_heap->tail = left;
    }
    mem_set((uint8_t*)va_node, 0x0, sizeof(vm_node_t) + va_node->no_bytes);
  }

  /* Merge with right if possible */ 
  if (k_heap->head->next != k_heap->tail && va_node->next != NULL && (va_node->next->flags & VM_USED) == 0) {
    vm_node_t* right = va_node->next;
    va_node->no_bytes += (sizeof(vm_node_t) + right->no_bytes);
    va_node->next = right->next;
    if (right->next != NULL) {
      right->next->prev = va_node;
    }
    if (k_heap->tail == right) {
      k_heap->tail = va_node;
    }
    mem_set((uint8_t*)right, 0x0, sizeof(vm_node_t) + right->no_bytes);
  }

  return 0;
};

/*

User space

*/

uintptr_t malloc(uint32_t no_bytes) {
  return 0;
}

void free() {
};

/*

VMM

*/

/*
Initialize a VM region with ~8KiB of memory.
Return pointer to head of VM range, and 0 if not possible.
*/
vm_range_t* vmm_init(uintptr_t vm_range_start){
  uintptr_t allocated_frame_head = alloc_frame(); 
  uintptr_t allocated_frame_tail = alloc_frame(); 
  if (!allocated_frame_head || !allocated_frame_tail) {
    return 0;
  }

  vm_range_t* vm_range = (vm_range_t*)vm_range_start;

  create_pte((uintptr_t)vm_range, allocated_frame_head);
  vm_range->head = (vm_node_t*)(vm_range_start + sizeof(vm_range_t*) + sizeof(vm_range_t));

  /* Head */

  uint32_t available_bytes = PAGE_SIZE - sizeof(vm_range_t*) - sizeof(vm_range_t) - sizeof(vm_node_t);
  vm_node_t* new_vm_range = vm_range->head;
  create_pte((uintptr_t)new_vm_range, allocated_frame_head);

  new_vm_range->prev = NULL;
  new_vm_range->next = NULL;
  new_vm_range->no_bytes = available_bytes;
  new_vm_range->flags = 0x0;

  /* Tail */

  available_bytes = PAGE_SIZE - sizeof(vm_node_t);
  new_vm_range = (vm_node_t*)((uintptr_t)(vm_range->head) + sizeof(vm_node_t) + vm_range->head->no_bytes);
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
uint8_t vmm_alloc(vm_range_t* vm_range, uint8_t flags, uintptr_t arg){
  uintptr_t allocated_frame = 0x0;
  if (flags & VM_MMIO){
    allocated_frame = arg;
  } else {
    allocated_frame = alloc_frame(); 
    if (!allocated_frame) {
      return 1;
    }
  }

  uint32_t available_bytes = PAGE_SIZE - sizeof(vm_node_t);
  vm_node_t* new_vm_range = (vm_node_t*)((uintptr_t)(vm_range->tail) + sizeof(vm_node_t) + vm_range->tail->no_bytes);
  create_pte((uintptr_t)new_vm_range, allocated_frame);
  create_pte((uintptr_t)new_vm_range + PAGE_SIZE - 1, allocated_frame);

  new_vm_range->prev = vm_range->tail;
  new_vm_range->next = NULL;
  new_vm_range->no_bytes = available_bytes;
  new_vm_range->flags = flags;

  vm_range->tail->next = new_vm_range;
  vm_range->tail = new_vm_range;

  /* Merge nodes (only if new node follows another free node) */
  if (vm_range->head->next != vm_range->tail && (vm_range->tail->prev->flags & VM_USED) == 0) {
    vm_node_t* left = vm_range->tail->prev;
    left->no_bytes += (sizeof(vm_node_t) + vm_range->tail->no_bytes);
    left->next = NULL;
    mem_set((uint8_t*)vm_range->tail, 0x0, sizeof(vm_node_t) + vm_range->tail->no_bytes);
    vm_range->tail = left;
  }

  return 0;
}

/* Cleanup VMM that is no longer in use */
void vmm_destroy(vm_range_t* vm_range) {}
