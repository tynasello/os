#include "include/io.h"
#include "include/screen.h"
#include "include/system.h"
#include <stdint.h>

/*
 
The Video Graphics Array (VGA) controller is responsible for managing video display output.  
It is responsible for rendering characters to the screen, controlling the cursor position, 
setting display modes (text mode, graphics mode), and handling various attributes like colors
and resolution.

In text mode, video memory is mapped to specific addresses in memory (starting at 0xb8000), 
and writing to these addresses updates what appears on the screen. 

Each character on the screen is represented by two bytes: 
one for the character itself (ASCII code) and one for its attribute (such as color).
Bits 4-7 of the color are the background, and bits 0-3 are the foreground.

The VGA controller receives signals through specific I/O ports.
CTRL is the control register port and is used to select a register to operate on:
- reg 14 is the high byte of the cursors offset
- reg 15 is the low byte of the cursors offset
DATA is the data register port used for transferring data to the controller.

*/

#define CTRL 0x3D4 
#define DATA 0x3D5 
#define CURSOR_HIGH 14
#define CURSOR_LOW 15

#define TXT_BUF_BASE 0xC00B8000

#define ROW_START 2 // Qemu doesn't display first two rows
#define ROW_END 25
#define COL_START 0
#define COL_END 80

typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t* vidmem;
  uint8_t* vidmem_backup;
  uint8_t* pos_backup;
} screen;

screen s = {
  .x = COL_START,
  .y = 9,
  .vidmem = (uint8_t *)TXT_BUF_BASE,
  .vidmem_backup = (uint8_t*)(TXT_BUF_BASE + COL_END * 2 * ROW_END),
  .pos_backup = (uint8_t*)(TXT_BUF_BASE + COL_END * 2 * ROW_END * 2),
};

static int get_offset(int x, int y) {
  return (y * COL_END + x) * 2; 
}

static uint8_t get_char_attr(VGATextColor fg, VGATextColor bg){
  return (bg << 4) | (fg & 0xF);
}

/*
 
Printing 

*/

void enable_cursor() {
	port_byte_out(CTRL, 0x0A);
	port_byte_out(DATA, (port_byte_in(DATA) & 0xC0) | 13);
	port_byte_out(CTRL, 0x0B);
	port_byte_out(DATA, (port_byte_in(DATA) & 0xE0) | 14);
}

void disable_cursor() {
	port_byte_out(CTRL, 0x0A);
	port_byte_out(DATA, 0x20);
}

static void set_cursor(int x, int y) {
  int offset = get_offset(x, y) / 2;
  port_byte_out(CTRL, 0xE);
  port_byte_out(DATA, (offset >> 8) & 0xFF);
  port_byte_out(CTRL, 0xF);
  port_byte_out(DATA, (offset) & 0xFF);
}

static void handle_scrolling() {
  if (s.y < ROW_END){
    return;
  }

  for (int y = 1; y < ROW_END; y++) {
    mem_cpy(get_offset(COL_START, y) + s.vidmem, get_offset(COL_START, y-1) + s.vidmem, COL_END * 2);
  }

  mem_set(get_offset(COL_START, ROW_END - 1) + s.vidmem, 0, COL_END * 2);
  s.x = COL_START;
  s.y = ROW_END - 1;
}

/*
  
Word

*/

static void print_char(char c, VGATextColor fg, VGATextColor bg) {
  if (c == '\n') {
    s.y++;
    s.x = COL_START;
  } else {
    int offset = get_offset(s.x, s.y);
    *(s.vidmem + offset) = c;
    *(s.vidmem + offset + 1) = get_char_attr(fg, bg);
    s.x++;
  }

  if (s.x == COL_END) {
    s.y++;
    s.x = COL_START;
  }

  handle_scrolling();
}

void print_at(const char* a, int x, int y, VGATextColor fg, VGATextColor bg) {
  y += ROW_START;
  if (x >= COL_START && x < COL_END) {
    s.x = x;
  }
  if (y >= ROW_START && y < ROW_END) {
    s.y = y;
  }

  int i = 0;
  while (a[i] != 0) {
    print_char(a[i++], fg, bg);
  }
  set_cursor(s.x, s.y);
}

void print(const char* a) { print_at(a, -1, -1, WHITE, BLACK); }

/*
  
Number

*/

void print_int_at(uint32_t num, int x, int y, VGATextColor fg, VGATextColor bg) {
  char num_str[11]; // Buffer to store the converted string. Assume 32-bit integer (10 chars) and null terminator
  itos(num, num_str);
  print_at(num_str, x, y, fg, bg); 
}

void print_int(uint32_t num) {
  print_int_at(num, -1, -1, WHITE, BLACK);
}

void print_hex_at(uint32_t num, int x, int y, VGATextColor fg, VGATextColor bg) {
  char num_str[12]; // Assuming a 32-bit integer (8 chars) and null terminator and preceeding '0x' (3 chars)
  htos(num, num_str);
  print_at(num_str, x, y, fg, bg); 
}

void print_hex(uint32_t num) {
  print_hex_at(num, -1, -1, WHITE, BLACK);
}

/*
  
Screen

*/

int get_screen_w(){
  return COL_END - COL_START;
}

int get_screen_h(){
  return ROW_END - ROW_START;
}

void clear_screen() {
  for (int y = ROW_START; y < ROW_END; y++) {
    for (int x = COL_START; x < COL_END; x++) {
      print_at(" ", x, y, WHITE, BLACK);
    }
  }
  s.x = COL_START;
  s.y = ROW_START;
  set_cursor(s.x, s.y);
}

void screen_backup(){
  mem_cpy(s.vidmem, s.vidmem_backup, COL_END * ROW_END * 2);
  mem_set(s.pos_backup, s.x, 1);
  mem_set(s.pos_backup + 1, s.y, 1);
}

void screen_restore(){
  mem_cpy(s.vidmem_backup, s.vidmem, COL_END * ROW_END * 2);
  uint8_t* cursor_backup = s.pos_backup;
  s.x = *cursor_backup;
  s.y = *(cursor_backup + 1);
  set_cursor(s.x, s.y);
};

void print_square(int x, int y, VGATextColor color) {
  print_at(" ", x, y, color, color);
}

/* Print block from (x1, y1) - (x2, y2) inclusive */
void print_block(int x1, int y1, int x2, int y2, VGATextColor color){
  for (int x = x1; x <= x2; x++){
    for (int y = y1; y <= y2; y++){
      print_square(x, y, color);
    }
  }
}

