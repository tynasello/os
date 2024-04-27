/* 
---------------------

Physical Memory Manager

Manages and allocated RAM through a page frame allocater

- Each page frame is 4096 bytes
- Uses a bitmap to keep track of used/free pages
  - Consider implementing a frame stack (constant time allocation and freeing)
- The PMM does not guarantee specific or contigous frames

--------------------- 
*/

#include "include/screen.h"
#include "include/system.h"
#include <stdint.h>

#define FREE_START 0x100000
#define NO_FRAMES 3584 // 14 MiB (0x00100000-0x00EFFFFF)
#define FRAME_SIZE 4096
#define FRAME_MAP_BITS_PER_ROW 8

/* 
Bit map where each byte represents status of 8 contiguous frames.
0 = free, 1 = taken.
*/
uint8_t frame_map[NO_FRAMES]; 

void pmm_init() {
  mem_set(frame_map, 0x00, NO_FRAMES/FRAME_MAP_BITS_PER_ROW);
}

/*
Allocate and return a physcial address that is free for use.
Return 0 if no physcial memory is available.
*/
uintptr_t alloc_frame(){
  int row = 0;
  while(frame_map[row] == ((1 << FRAME_MAP_BITS_PER_ROW) - 1)){
    if(row == NO_FRAMES){
      return 0;
    }
    row++;
  }

  int col = 0;
  while(frame_map[row] & 1 << col){
    col++;
  }

  frame_map[row] = frame_map[row] | (1 << col);
  return FREE_START + (row * FRAME_MAP_BITS_PER_ROW + col) * FRAME_SIZE;
}

void free_frame(uint32_t phys_addr){
  int frame_map_loc = (phys_addr - FREE_START) / FRAME_SIZE;
  int col = frame_map_loc % FRAME_MAP_BITS_PER_ROW;
  int row = (frame_map_loc - col) / FRAME_MAP_BITS_PER_ROW;
  frame_map[row] = frame_map[row] & ~(1 << col);
}
