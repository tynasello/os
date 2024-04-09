#include "include/io.h"
#include "include/screen.h"
#include "include/system.h"
#include <stdint.h>

/*

I/O Ports:
- port to the screen controller's control register, used to select the operating register:
  - reg 14 is the high byte of the cursors offset
  - reg 15 is the low byte of the cursors offset
- port to the screen controller's data register, used to transfer data

*/
#define SCREEN_CTRL_PORT 0x3D4 
#define SCREEN_DATA_PORT 0x3D5 
#define SCREEN_REG_CURSOR_HIGH 14
#define SCREEN_REG_CURSOR_LOW 15

#define VIDEO_MEMORY 0xb8000 // Address of memory-mapped screen device
#define VIDEO_MEMORY_BACKUP VIDEO_MEMORY + MAX_ROWS * MAX_COLS * 2
#define MAX_ROWS 25
#define MAX_COLS 80

/* Bits 4-7 are background, bits 0-3 are foreground */
#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define MAGENTA 0x5
#define BROWN 0x6
#define LIGHT_GRAY 0x7
#define DARK_GRAY 0x8
#define LIGHT_BLUE 0x9
#define LIGHT_GREEN 0xA
#define LIGHT_CYAN 0xB
#define LIGHT_RED 0xC
#define LIGHT_MAGENTA 0xD
#define YELLOW 0xE
#define WHITE 0xF

char get_c_attr(char bg, char fg){
  return (bg << 4) | (fg & 0xF);
}

char color_to_attr(enum Color color){
  if(color == BLACKoBLACK){
    return get_c_attr(BLACK, BLACK);
  } else if(color == WHITEoBLACK){
    return get_c_attr(BLACK, WHITE);
  } else if(color == BLACKoWHITE){
    return get_c_attr(WHITE, BLACK);
  } else if(color == GREENoBLACK){
    return get_c_attr(GREEN, BLACK);
  } else if(color == REDoBLACK){
    return get_c_attr(RED, BLACK);
  } else{
    return get_c_attr(BLACK, WHITE);
  }
}

int get_screen_w(){
  return MAX_COLS;
}

int get_screen_h(){
  return MAX_ROWS;
}

int get_offset(int row, int col) {
  return (row * MAX_COLS + col) * 2; 
}

int handle_scrolling(int offset) {
  if (offset < MAX_ROWS * MAX_COLS * 2) {
    return offset;
  }
  char *vidmem = (char *)VIDEO_MEMORY;
  for (int row = 1; row < MAX_ROWS; ++row) {
    mem_cpy((unsigned char *)(get_offset(row, 0) + vidmem),
            (unsigned char *)(get_offset(row - 1, 0) + vidmem), MAX_COLS * 2);
  }
  char *last_line = get_offset(MAX_ROWS - 1, 0) + vidmem;
  for (int col = 0; col < MAX_COLS * 2; ++col) {
    last_line[col] = 0;
  }
  offset -= 2 * MAX_COLS;
  return offset;
}

int get_cursor() {
  /*
  A character cell is represented by two bytes:
  - the first is the ascii code for the character
  - the second is the character attributes (foreground, background, blinking)

  Cursor offset in VGA hardware is represented as the number of written characters (2 bytes each).
  */
  port_byte_out(SCREEN_CTRL_PORT, SCREEN_REG_CURSOR_HIGH);
  int offset = port_byte_in(SCREEN_DATA_PORT) << 8;
  port_byte_out(SCREEN_CTRL_PORT, SCREEN_REG_CURSOR_LOW);
  offset += port_byte_in(SCREEN_DATA_PORT);
  return offset * 2;
}

void set_cursor(int offset) {
  offset /= 2;
  port_byte_out(SCREEN_CTRL_PORT, 14);
  port_byte_out(SCREEN_DATA_PORT, offset >> 8);
  port_byte_out(SCREEN_CTRL_PORT, 15);
  port_byte_out(SCREEN_DATA_PORT, offset);
}

/*
  
Word

*/

static void print_char(char c, enum Color color) {
  char c_attr = color_to_attr(color);

  char *vidmem = (char *)VIDEO_MEMORY;
  int offset = get_cursor();
  if (c == '\n') {
    offset = get_offset(offset / (2 * MAX_COLS), MAX_COLS - 1);
  } else {
    *(vidmem + offset) = c;
    *(vidmem + offset + 1) = c_attr;
  }
  offset += 2;
  offset = handle_scrolling(offset);
  set_cursor(offset);
}

void print_at(const char *s, int row, int col, enum Color color) {
  if (row >= 0 && col >= 0) {
    set_cursor(get_offset(row, col));
  }
  int i = 0;
  while (s[i] != 0) {
    print_char(s[i++], color);
  }
}

void print(const char *s) { print_at(s, -1, -1, 0); }

/*
  
Number

*/

void print_int_at(uint32_t num, int row, int col, enum Color color) {
  char buffer[11]; // Buffer to store the converted string. Assume 32-bit integer and null terminator
  itos(num, buffer);
  print_at(buffer, row, col, color); 
}

void print_int(uint32_t num) {
  print_int_at(num, -1, -1, 0);
}

void print_hex_at(uint32_t num, int row, int col, enum Color color) {
  char buffer[12]; // Assuming a 32-bit integer and null terminator and preceeding '0x'
  htos(num, buffer);
  print_at(buffer, row, col, color); 
}

void print_hex(uint32_t num) {
  print_hex_at(num, -1, -1, 0);
}

/*
  
Screen

*/

void clear_screen() {
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      print_at(" ", row, col, 0);
    }
  }
  set_cursor(get_offset(2, 0)); // Should be 0 not 2; using 2 because qemu window too small
}

void screen_backup(){
  mem_cpy((unsigned char*)VIDEO_MEMORY, (unsigned char*)VIDEO_MEMORY_BACKUP, MAX_COLS * MAX_ROWS * 2);
  clear_screen();
}

void screen_restore(){
  clear_screen();
  mem_cpy((unsigned char*)VIDEO_MEMORY_BACKUP, (unsigned char*)VIDEO_MEMORY, MAX_COLS * MAX_ROWS * 2);
};

static int bound_screen(int a, int mi, int ma) {
    if (a < mi){
      return mi;
    }
    else if (a >= ma){
      return ma - 1;
    }
    else{
      return a;
    }
}

/* Print block from (x1, y1) - (x2, y2) inclusive */
void print_block(int x1, int x2, int y1, int y2, enum Color color){
  x1 = bound_screen(x1, 0, MAX_COLS);
  x2 = bound_screen(x2, 0, MAX_COLS);
  y1 = bound_screen(y1, 0, MAX_ROWS);
  y2 = bound_screen(y2, 0, MAX_ROWS);


  for (int x = x1; x <= x2; x++){
    for (int y = y1; y <= y2; y++){
      print_square(x, y, color);
    }
  }

}

void print_square(int x, int y, enum Color color){
  x = bound_screen(x, 0, MAX_COLS);
  y = bound_screen(y, 0, MAX_ROWS);

  print_at(" ", y, x, color);
}
