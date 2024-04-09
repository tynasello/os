#include <stdint.h>

/*

Memory

*/

void mem_cpy(uint8_t *source, uint8_t *dest, uint32_t no_bytes){
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = *(source + i);
  }
}

void mem_set(uint8_t *dest, uint8_t val, uint32_t no_bytes) {
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = val;
  }
}

/*

Number

*/

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

void itos(uint32_t a, char *s) {
  int i = 0;
  do {
    s[i++] = '0' + a % 10;
    a /= 10;
  } while (a > 0);
  s[i] = '\0';
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = s[start];
    s[start] = s[end];
    s[end] = temp;
    start++;
    end--;
  }
}

void htos(uint32_t a, char *s) {
  int i = 0;
  do {
    int remainder = a % 16;
    s[i++] = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
    a /= 16;
  } while (a > 0);

  s[i++] = 'x';
  s[i++] = '0';
  s[i] = '\0';
  // Reverse the string in-place
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = s[start];
    s[start] = s[end];
    s[end] = temp;
    start++;
    end--;
  }
}
