[bits 16]

GDT_START:; Start address of the GDT, defined below

GDT_NULL: ; Manditory null descriptor
  dd 0x0  ; dd <=> display double word (32 bits); dw <=> display word (16 bits)
  dd 0x0

GDT_CODE: ; Code segment descriptor
  ; Base        <=> 0x0    
  ; Limit       <=> 0xfffff (granularity set as 1 multiples by 4K, become 0xfffff000)
  ; Type flags  <=> 1010b   <=> 1(code) 0(conforming) 1(readable) 0(accessed)
  ; 1st flags   <=> 1001b   <=> 1(segment present) 00(privilege ring) 1(descriptor type)
  ; 2nd flags   <=> 1100b   <=> 1(granularity; multiply limit by: 0=1byte, 1=4kbyte) 1(32-bit default operation size) 0(64-bit code segment) 0(AVL)
  dw 0xffff     ; Limit (low)   (bits 0-15)
  dw 0x0        ; Base (low)    (bits 16-31)
  db 0x0        ; Base (middle) (bits 32-39)
  db 10011010b  ; Type flags    (bits 40-43),   1st flags (bits 44-47)
  db 11001111b  ; Limit (high)  (bits 48-51),   2nd flags (bits 52-55)
  db 0x0        ; Base (high)   (bits 56-63)


GDT_DATA: ; Data segment descriptor
  ; Same as code segment descriptor except for the type flags:
  ; Type flags  <=> 0010b   <=> 0(code) 0(expand down) 1(writable) 0(accessed)
  dw 0xffff     
  dw 0x0        
  db 0x0        
  db 10010010b  
  db 11001111b  
  db 0x0       


GDT_END:  ; End address of the GDT, defined above


; GDT descriptor is what the CPU needs: 
; GDT size (bits 0-15), GDT address (bits 16-47)
GDT_DESCRIPTOR: 
  dw GDT_END - GDT_START - 1      ; Size is always one less than true size?
  dd GDT_START

; Define offsets (indexes) into GDT that will be used when setting segment registers
CODE_SEG equ GDT_CODE - GDT_START ; equ defines symbolic constants or aliases and is a feature provided by the assembler
DATA_SEG equ GDT_DATA - GDT_START ; 0x0 -> NULL; 0x08 -> CODE; 0x10 -> DATA 
