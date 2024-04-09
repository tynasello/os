#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stdint.h>

void mem_cpy(uint8_t *source, uint8_t *dest, uint32_t no_bytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t no_bytes);
uint32_t rand_range(uint32_t min, uint32_t max);
void itos(uint32_t a, char* s);
void htos(uint32_t a, char* s);

#endif 
