#ifndef __VMM_H
#define __VMM_H

#include "stdint.h"

void vm_init();
uintptr_t kmalloc(uint32_t no_bytes);
int kfree(void* va);

#endif
