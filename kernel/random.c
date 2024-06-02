#include <stdint.h>

static uint32_t rdtsc() {
  uint32_t low, high;
  __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high));
  return low;
}

static const uint32_t a = 1664525;
static const uint32_t c = 1013904223;
static const uint32_t m = 4294967295; // 2^32-1 (avoid overflows)

static uint32_t rand() { return (a * rdtsc() + c) % m; }

/* Generate a psuedo-random number in the range [min, max] */
uint32_t rand_range(uint32_t min, uint32_t max) {
  return min + (rand() % (max - min + 1));
}
