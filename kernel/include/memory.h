#ifndef __MEMORY_H
#define __MEMORY_H

#include <stdint.h>

void mem_cpy(uint8_t *source, uint8_t *dest, uint32_t no_bytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t no_bytes);

#endif
