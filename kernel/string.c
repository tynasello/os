#include <stdint.h>

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
