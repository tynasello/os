#ifndef __SYSTEM_H
#define __SYSTEM_H

void mem_cpy(const unsigned char *source, unsigned char *dest, int no_bytes);
void mem_set(unsigned char *dest, unsigned char val, int no_bytes);
void mem_set_word(unsigned short *dest, unsigned short val, int no_words);

#endif 
