#ifndef __SCREEN_H
#define __SCREEN_H

#include <stdint.h>

typedef enum {
  BLACK = 0,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  LIGHT_GRAY,
  DARK_GRAY,
  LIGHT_BLUE,
  LIGHT_GREEN,
  LIGHT_CYAN,
  LIGHT_RED,
  LIGHT_MAGENTA,
  LIGHT_BROWN,
  WHITE,
} VgaTextColor;

void enable_cursor();
void disable_cursor();

void print_at(const char *a, int x, int y, VgaTextColor fg, VgaTextColor bg);
void print(const char *a);

void print_int_at(uint32_t num, int x, int y, VgaTextColor fg, VgaTextColor bg);
void print_int(uint32_t num);
void print_hex_at(uint32_t num, int x, int y, VgaTextColor fg, VgaTextColor bg);
void print_hex(uint32_t num);

void clear_screen();
void screen_restore();
void screen_backup();
int get_screen_w();
int get_screen_h();
void print_square(int x, int y, VgaTextColor color);
void print_block(int x1, int y1, int x2, int y2, VgaTextColor color);

#endif
