#ifndef __SCREEN_H
#define __SCREEN_H

void print(const char *s);
void print_at(const char *s, int row, int col);
void print_char(char c, char c_attr);
void clear_screen();
void print_int(int num);
void print_hex(int num);

#endif
