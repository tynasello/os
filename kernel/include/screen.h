#ifndef __SCREEN_H
#define __SCREEN_H

#include <stdint.h>

typedef enum Color{
  WHITEoBLACK,
  BLACKoWHITE,
  BLACKoBLACK,
  GREENoBLACK,
  REDoBLACK,
} Color;

void screen_backup();
void screen_restore();
char get_c_attr(char bg, char fg);
int get_screen_w();
int get_screen_h();
void print(const char *s);
void print_at(const char *s, int row, int col, enum Color color);
void print_int_at(uint32_t num, int row, int col, enum Color color);
void print_int(uint32_t num);
void print_hex_at(uint32_t num, int row, int col, enum Color color);
void print_hex(uint32_t num);
void clear_screen();
void print_block(int x1, int x2, int y1, int y2, enum Color color);
void print_square(int x, int y, enum Color color);

#endif
