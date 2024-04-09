#include "include/io.h"
#include "include/irq.h"
#include "include/screen.h"

#define NUM_OBSERVERS 5
/*
Anything the keyboard is trying to send to the computer is sent through the data register. 
The keyboard raises IRQ1 when it has data available for reading.
*/
#define KB_DATA_PORT 0x60

/* Keyboard scancode to ASCII lookup table using US Keyboard Layout */
unsigned char kb_us_keymap[128];

/* 
Register observers to call when keyboard IRQ fired.
Return 0 if registration unsuccessful.
*/
static void* observers[NUM_OBSERVERS];

int register_kb_observer(void* fn){
  int i;
  for(i = 0; i < NUM_OBSERVERS; i++){
    if(observers[i] == 0){
      break;
    }
  }
  if (i >= NUM_OBSERVERS){
    return 0;
  }

  observers[i] = fn;
  return 1;
}

void deregister_kb_observer(void* fn){
  for(int i = 0; i < NUM_OBSERVERS; i++){
    if(observers[i] == fn){
      observers[i] = 0;
    }
  }
}


/*
Keyboard IRQ handler
*/
void kb_handler(struct regs *r) {
  unsigned char scancode = port_byte_in(KB_DATA_PORT);

  if (scancode & 0x80) {
    // Key has been released if top bit of byte is set
  } else {
    // Key has been pressed
    
    char c = kb_us_keymap[scancode];
    
    for (int i = 0; i < NUM_OBSERVERS; i++){
      if(observers[i] == 0){
        break;
      }
      void (*fn)(char c); 
      fn = observers[i];
      fn(c);
    }
  }
}

/*
 
Add the kb handler to the IRQ based interrupt mapping

*/
void kb_install() {

  kb_us_keymap[0] = 0;
  kb_us_keymap[1] = 27;
  kb_us_keymap[2] = '1';
  kb_us_keymap[3] = '2';
  kb_us_keymap[4] = '3';
  kb_us_keymap[5] = '4';
  kb_us_keymap[6] = '5';
  kb_us_keymap[7] = '6';
  kb_us_keymap[8] = '7';
  kb_us_keymap[9] = '8';
  kb_us_keymap[10] = '9';
  kb_us_keymap[11] = '0';
  kb_us_keymap[12] = '-';
  kb_us_keymap[13] = '=';
  kb_us_keymap[14] = '\b';
  kb_us_keymap[15] = '\t';
  kb_us_keymap[16] = 'q';
  kb_us_keymap[17] = 'w';
  kb_us_keymap[18] = 'e';
  kb_us_keymap[19] = 'r';
  kb_us_keymap[20] = 't';
  kb_us_keymap[21] = 'y';
  kb_us_keymap[22] = 'u';
  kb_us_keymap[23] = 'i';
  kb_us_keymap[24] = 'o';
  kb_us_keymap[25] = 'p';
  kb_us_keymap[26] = '[';
  kb_us_keymap[27] = ']';
  kb_us_keymap[28] = '\n';
  kb_us_keymap[29] = 0;      // Control
  kb_us_keymap[30] = 'a';
  kb_us_keymap[31] = 's';
  kb_us_keymap[32] = 'd';
  kb_us_keymap[33] = 'f';
  kb_us_keymap[34] = 'g';
  kb_us_keymap[35] = 'h';
  kb_us_keymap[36] = 'j';
  kb_us_keymap[37] = 'k';
  kb_us_keymap[38] = 'l';
  kb_us_keymap[39] = ';';
  kb_us_keymap[40] = '\'';
  kb_us_keymap[41] = '`';
  kb_us_keymap[42] = 0;     // Left shift
  kb_us_keymap[43] = '\\';
  kb_us_keymap[44] = 'z';
  kb_us_keymap[45] = 'x';
  kb_us_keymap[46] = 'c';
  kb_us_keymap[47] = 'v';
  kb_us_keymap[48] = 'b';
  kb_us_keymap[49] = 'n';
  kb_us_keymap[50] = 'm';
  kb_us_keymap[51] = ',';
  kb_us_keymap[52] = '.';
  kb_us_keymap[53] = '/';
  kb_us_keymap[54] = 0;     // Right shift
  kb_us_keymap[55] = '*';
  kb_us_keymap[56] = 0;     // Alt
  kb_us_keymap[57] = ' ';
  kb_us_keymap[58] = 0;     // Caps lock
  kb_us_keymap[59] = 0;     // F1 key
  kb_us_keymap[60] = 0;     // F2 key
  kb_us_keymap[61] = 0;     // F3 key
  kb_us_keymap[62] = 0;     // F4 key
  kb_us_keymap[63] = 0;     // F5 key
  kb_us_keymap[64] = 0;     // F6 key
  kb_us_keymap[65] = 0;     // F7 key
  kb_us_keymap[66] = 0;     // F8 key
  kb_us_keymap[67] = 0;     // F9 key
  kb_us_keymap[68] = 0;     // F10 key
  kb_us_keymap[69] = 0;     // Num lock
  kb_us_keymap[70] = 0;     // Scroll Lock
  kb_us_keymap[71] = 0;     // Home key
  kb_us_keymap[72] = 0;     // Up Arrow
  kb_us_keymap[73] = 0;     // Page Up
  kb_us_keymap[74] = '-';
  kb_us_keymap[75] = 0;     // Left Arrow
  kb_us_keymap[76] = 0;
  kb_us_keymap[77] = 0;     // Right Arrow
  kb_us_keymap[78] = '+';
  kb_us_keymap[79] = 0;     // End key
  kb_us_keymap[80] = 0;     // Down Arrow
  kb_us_keymap[81] = 0;     // Page Down
  kb_us_keymap[82] = 0;     // Insert Key
  kb_us_keymap[83] = 0;     // Delete Key
  kb_us_keymap[84] = 0;
  kb_us_keymap[85] = 0;
  kb_us_keymap[86] = 0;
  kb_us_keymap[87] = 0;     // F11 Key
  kb_us_keymap[88] = 0;     // F12 Key
                            // Additional keys are undefined

  irq_install_handler(1, kb_handler);
}
