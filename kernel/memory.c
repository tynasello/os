#include <stdint.h>

void mem_cpy(uint8_t *source, uint8_t *dest, uint32_t no_bytes) {
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = *(source + i);
  }
}

void mem_set(uint8_t *dest, uint8_t val, uint32_t no_bytes) {
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = val;
  }
}
