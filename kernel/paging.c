#include "include/pmm.h"
#include "include/screen.h"
#include <stdint.h>

typedef struct {
  uint32_t entries[1024];
} page_table_t;

typedef struct {
  uint32_t entries[1024];
} page_directory_t;

page_directory_t *page_directory;

/*

Initialize page directory and page tables.
Enable paging.

*/
void page_init() {
  page_directory = (page_directory_t *) kalloc_frame();

  /* Initialize page table directory */
  for (int i = 0; i < 1024; i++) {
    /*
    The following flags are applied to each page:
    Supervisor: Only kernel-mode can access them,
    Write Enabled: It can be both read from and written to,
    Not Present: The page table is not present.

    MMU issues a page fault if it encounters a non-present page
    */
    page_directory->entries[i] = 0x2;
  }

  /* Initialize first page table */
  page_table_t *page_table = (page_table_t *) kalloc_frame();
  for (int i = 0; i < 1024; i++) {
    /* Attributes: supervisor level, read/write, present. */
    page_table->entries[i] = (i*0x1000) | 0x3;
  }

  /* Page is now present */
  page_directory->entries[0] = (int)page_table | 0x3;

  /* Load page directory */
  asm volatile("mov %0, %%cr3" :: "r"(page_directory): "memory");

  /* Enable paging */
  uint32_t cr0;
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 = cr0 | 0x80000000;
  asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
