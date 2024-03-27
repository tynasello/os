/* 
---------------------

Physical Memory Manager
- Handles RAM allocation through a page frame allocater

--------------------- 
*/

#include "include/screen.h"
#include "include/system.h"
#include <stdint.h>

#define FREE_START 0x100000
#define NUMFRAMES 3584 // 14 MiB (0x00100000-0x00EFFFFF)
#define FREE 1
#define TAKEN 0
#define FRAME_SIZE 4096

/* Set in linker script */
extern uint32_t endkernel;

/* Bit map where each byte represents status of 8 contiguous pages */
unsigned char frame_map[NUMFRAMES]; 

void pmm_init() {
  mem_set(frame_map, 0xff, NUMFRAMES/8);
}

void print_endkernel(){
  print_hex((uintptr_t)&endkernel);
}

/* 
Current method used for simplicity.
Alternative is to use a stack structure.
*/
uintptr_t alloc_frame(){
  int i = 0;
  while(frame_map[i] == TAKEN){
    if(i == NUMFRAMES){
      return 0;
    }
    i++;
  }
  int j = 0;
  while((frame_map[i] & (FREE << j)) == TAKEN){
    if(j == 8){
      return 0;
    }
    j++;
  }

  frame_map[i] = frame_map[i] & ~(FREE << j);
  return FREE_START + (i * 8 + j) * FRAME_SIZE;
}
