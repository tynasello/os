/*
--------------------- 

Virtual Memory Manager

Setup paging and create two memory managers: 
- one to implement malloc() and free() for the kernel
- one to manage the virtual address space for user applications

--------------------- 
*/ 

#include "include/pmm.h"
#include "include/screen.h"
#include <stdint.h>

#define PAGE_SIZE 4096
#define PD_SIZE 1024
#define PT_SIZE 1024
#define PD_RECURSIVE_I (PD_SIZE - 1)
#define PAGE_FLAG_INVALID 0x0
#define PAGE_FLAG_PRESENT 0x1
#define K_PAGE_FLAG_INIT 0x3

#define VA_PDI_START 22
#define VA_PTI_START 12

#define KHEAP_START 0xD0000000
#define KHEAP_END 0xE0000000

/*
Page table entry format:
- bit 0: present: page is currently in memory.
- bit 1: write enabled: can be both read from and written to
- bit 2: user-accessible: if not set, only kernel mode code can access this page
- bit 3: write-through
- bit 4: cache-disable
- bit 5: accessed
- bit 6: dirty: page has been written to
- bit 7: page attribute table
- bit 8: global
- bit 9-11: free for use by OS
- bit 12-31: bits 12-31 of physical address of a page frame

Page tables (and directory) must be 4KiB aligned.
*/
typedef struct {
  uintptr_t frames[PT_SIZE]; // Each entry is a (physical) address of a page frame
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
  pt_t* pts[PD_SIZE]; // Each entry is a physical address to a page table
} pd_t;

pd_t* k_pd;
pd_t* k_pd_va;

/*

Initialize page directory and page table.
Enable paging.

*/
void vmm_init() {
  k_pd = (pd_t *) alloc_frame();
  k_pd_va = (pd_t*)(PD_RECURSIVE_I<<22);

  /* Initialize page table directory */
  for(int i = 0; i < PD_SIZE; i++) {
    k_pd->pts[i] = (pt_t*)PAGE_FLAG_INVALID; // Access will cause page fault
  }

  /*
  Identity-map the real-mode address space (< 1MiB),
  this way kernel code, stack, and video memory are still accessible using same addresses after paging is enabled. 
  */
  pt_t *pt = (pt_t *) alloc_frame();
  for(int i = 0; i < PD_SIZE; i++) {
    if(i * PAGE_SIZE <= 0xFF000){
      pt->frames[i] = i * PAGE_SIZE | K_PAGE_FLAG_INIT;
    }else{
      pt->frames[i] = PAGE_FLAG_INVALID;
    }

  }

  /* Add first page table to page directory*/
  k_pd->pts[0] = (pt_t*)((uintptr_t)pt | K_PAGE_FLAG_INIT);

  /* 
  To access page table frames, we need to map virtual addresses to them. 
  The following recursively maps the page directory.
  */
  k_pd->pts[PD_RECURSIVE_I] = (pt_t *)((uintptr_t)k_pd | K_PAGE_FLAG_INIT);

  /* 
  Load page directory.

  cr3 is the page directory base register.
  On each memory access, the CPU reads the table pointer from this register
  and looks up the mapped frame for a given virtual address.
  The CPU uses the MMU to automatically walks the page tables and caches the 
  resulting translations in the translation lookaside buffer (TLB). 
  This buffer is not updated transparently and needs to be flushed manually
  on page directory or table changes.
  The INVLPG instruction instructs the processor to invalidate only the 
  region of the TLB associated with one page.

  The OS can change cr3, and in doing so, different page directories can be 
  used to give separate processes/applications different address spaces.

  MMU issues a page fault if it encounters a non-present page
  */
  asm volatile("mov %0, %%cr3" :: "r"(k_pd): "memory");

  /* Enable paging */
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 = cr0 | 0x80000000;
  asm volatile("mov %0, %%cr0" :: "r"(cr0));
}


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

Kernel virtual memory is split up into 256MB-regions as follows:
- 0xC0000000	Kernel code, data, bss, etc (currently not using this)
- 0xD0000000	Kernel heap
- 0xE0000000	Space reserved for device drivers
- 0xF0000000	Some physical memory (useful for video memory access). 
              Page directory and page tables of the current process

x86 virtual address format:
- bits 22-31: page directory index
- bits 12-21: page table index
- bits 0-11: offset within the page

*/

static uint32_t va_to_pde(uint32_t va){
  return va >> VA_PDI_START & 0x3FF;
}

static uint32_t va_to_pte(uint32_t va){
  return va >> VA_PTI_START & 0x3FF;
}

static int pde_nexists(uintptr_t va){
  int pde_i = va_to_pde(va);
  return ((uintptr_t)(((pd_t*)((uintptr_t)k_pd_va | PD_RECURSIVE_I << VA_PTI_START))->pts[pde_i]) & PAGE_FLAG_PRESENT) == 0;
}

static int pte_free(uintptr_t va){
  int pde_i = va_to_pde(va);
  int pte_i = va_to_pte(va);
  if(pde_nexists(va)){
    return 1;
  }
  return ((uintptr_t)(((pt_t*)((uintptr_t)k_pd_va | pde_i << VA_PTI_START))->frames[pte_i]) & PAGE_FLAG_PRESENT) == 0;
}

static void create_pde(uintptr_t va){
  int pde_i = va_to_pde(va);
  (((pd_t*)((uintptr_t)k_pd_va | PD_RECURSIVE_I << VA_PTI_START)))->pts[pde_i] = (pt_t*)((uintptr_t)alloc_frame() | K_PAGE_FLAG_INIT);
}

static void create_pte(uintptr_t va, uintptr_t frame){
  int pde_i = va_to_pde(va);
  int pte_i = va_to_pte(va);
  (((pt_t*)((uintptr_t)k_pd_va | pde_i << VA_PTI_START)))->frames[pte_i] = (uintptr_t)(frame | K_PAGE_FLAG_INIT);
}

uintptr_t kmalloc(int no_bytes){
  if(!no_bytes){
    return 0;
  }

  int no_pages = (no_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

  /* Find available VA range */
  
  uint32_t va;
  uint32_t found = 0;

  for(va = KHEAP_START; va < KHEAP_END;){
    uint32_t start = va;
    while(start < KHEAP_END){ // kmalloc() should call morecore() when it runs out of memory
      if((start - va) / PAGE_SIZE == no_pages){
        found = 1;
        break;
      }
      if(!pte_free(start)){
        va = start + PAGE_SIZE;
        break;
      }
      start += PAGE_SIZE;
    }
    if(found){
      break;
    }
  }

  if(!va || va >= KHEAP_END || !found){
    return 0;
  }

  /* Allocate PA range for VA range */
  
  for(int i = 0; i < no_pages; i++){
    uintptr_t frame = alloc_frame();
    if(!frame){
      return 0;
    }
    if(pde_nexists(va + i * PAGE_SIZE)){
      create_pde(va + i * PAGE_SIZE);
    }
    create_pte(va + i * PAGE_SIZE, frame);
  }

  return va;
}
