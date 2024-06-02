#ifndef __VMM_H
#define __VMM_H

#define NO_PDE 1024
#define NO_PTE 1024

#include "stdint.h"

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
  uintptr_t
      frames[NO_PTE]; // Each entry is a (physical) address to a page frame
} Pt;

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
  Pt *pts[NO_PDE]; // Each entry is a (physical) address to a page table
} Pd;

/* A linked list representing all the current page directories */
typedef struct ProcessPd {
  Pd *pd_va;
  uintptr_t pd_pa;
  struct ProcessPd *next;
} ProcessPd;

ProcessPd *create_process_pd();
void delete_process_pd(ProcessPd *process_pd);
void load_pd(uintptr_t pd_pa);
void vm_init();
uintptr_t kmalloc(uint32_t no_bytes);
int kfree(void *va);

#endif
