#include "include/io.h"
#include "include/system.h"

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
#define MAX_ROWS 25
#define MAX_COLS 80
#define TEXT_ATTR 0x07 // Bits 4-7 are background, bits 0-3 are foreground


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

void print_char(char c, char c_attr) {
  if (!c_attr) {
    c_attr = TEXT_ATTR;
  }
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

void print_at(const char *s, int row, int col) {
  if (row >= 0 && col >= 0) {
    set_cursor(get_offset(row, col));
  }
  int i = 0;
  while (s[i] != 0) {
    print_char(s[i++], TEXT_ATTR);
  }
}

void print(const char *s) { print_at(s, -1, -1); }

void clear_screen() {
  for (int row = 0; row < MAX_ROWS; ++row) {
    for (int col = 0; col < MAX_COLS; ++col) {
      print_at(" ", row, col);
    }
  }
  set_cursor(get_offset( 2, 0)); // Should be 0 not 2; using 2 because qemu window too small
}

void print_int(int num) {
  char buffer[10]; // Buffer to store the converted string. Assume 32-bit integer, null terminator, and sign.
  int is_negative = 0; // Handle negative numbers
  if (num < 0) {
    is_negative = 1;
    num = -num;
  }
  int i = 0;
  do {
    buffer[i++] = '0' + num % 10;
    num /= 10;
  } while (num > 0);

  if (is_negative) {
    buffer[i++] = '-';
  }
  buffer[i] = '\0'; 
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }
  print(buffer);
}

void print_hex(int num) {
  char buffer[10]; // Assuming a 32-bit integer and null terminator and preceeding '0x'
  int i = 0;
  do {
    int remainder = num % 16;
    buffer[i++] = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
    num /= 16;
  } while (num > 0);

  buffer[i++] = 'x';
  buffer[i++] = '0';
  buffer[i] = '\0'; 
  // Reverse the string in-place
  int start = 0;
  int end = i - 1;
  while (start < end) {
    char temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }
  print(buffer);
}
