#include "include/screen.h"
#include "include/system.h"
#include <stdint.h>

/* Retrieved from linker. Memory after this point is free to use. */
extern uint32_t endkernel;
uint32_t phys_start;

/*
Each byte represents status of 8 pages
0 = taken
1 = free
*/
unsigned char frame_map[128]; 

void pmm_init() {
  phys_start = (int)&endkernel - ((int)&endkernel % 4096) + 4096;
  mem_set(frame_map, 0xff, 128);
}

void print_endkernel(){
  print_hex((int)&endkernel);
}

int kalloc_frame(){
  int i = 0;
  while(frame_map[i] == 0){
    if(i == 128){
      return -1;
    }
    i++;
  }
  int j = 0;
  while((frame_map[i] & (1 << j)) == 0){
    if(j == 8){
      return -1;
    }
    j++;
  }

  frame_map[i] = frame_map[i] & ~(1 << j);
  return phys_start + (i * 8 + j) * 4096;
}
