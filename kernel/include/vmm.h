#ifndef __VMM_H
#define __VMM_H

#include "stdint.h"

void vmm_init();
uintptr_t kmalloc(int no_bytes);

#endif
