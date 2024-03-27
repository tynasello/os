#ifndef __SCREEN_H
#define __SCREEN_H

#include <stdint.h>

void print(const char *s);
void print_at(const char *s, int row, int col);
void print_char(char c, char c_attr);
void clear_screen();
void print_int(uint32_t num);
void print_hex(uint32_t num);

#endif
