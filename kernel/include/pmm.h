#ifndef __PMM_H
#define __PMM_H

#include "stdint.h"

void pmm_init();
uintptr_t alloc_frame();
void free_frame(uintptr_t phys_addr);

#endif
