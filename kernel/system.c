void mem_cpy(const unsigned char *source, unsigned char *dest, int no_bytes) {
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = *(source + i);
  }
}

void mem_set(unsigned char *dest, unsigned char val, int no_bytes) {
  for (int i = 0; i < no_bytes; ++i) {
    *(dest + i) = val;
  }
}

void mem_set_word(unsigned short *dest, unsigned short val, int no_words) {
  for (int i = 0; i < no_words; i += 1) {
    *(dest + i) = val;
  }
}
