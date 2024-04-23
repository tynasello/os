%include "rm_print.asm"

KERNEL_OFFSET equ 0x500 ; Offset where the kernel will be loaded 
CODE_SEG equ gdt_code - gdt_start ; Define offsets (indexes) into GDT that will be used when setting segment registers
DATA_SEG equ gdt_data - gdt_start ; 0x0 -> NULL; 0x08 -> CODE; 0x10 -> DATA 

start_rm:
	mov [boot_drive], dl 	; BIOS detects available storage devices
												; The location of the boot drive is stored in dl, hold on to this...
	mov bx, msg_rm
	call rm_print

load_kernel:
	mov bx, msg_load
	call rm_print
	mov bx, KERNEL_OFFSET ; Load 40 sectors (512B each) to 0x500 (ES:KERNEL_OFFSET)
	mov dh, 40
	mov dl, [boot_drive]

disk_load:
  push dx
  mov ah, 0x02      ; Select BIOS read sector function
  mov al, dh        ; Number of sectors to read
  mov ch, 0x00      ; Read from cylinder 0
  mov dh, 0x00      ; Read from head 0
  mov cl, 0x02      ; Read from the second sector (first sector is the boot sector)
  int 0x13          ; BIOS interrupt for disk access
  jc disk_err       ; A set carry flag indicates error
  pop dx
  cmp dh, al        ; al <=> number of sectors read; dh <=> number of sectors requested to be read
  jne disk_err 

prepare_pm:
  cli               ; Disable interrupts for now (current set-up won't work in PM)
  lgdt [gdt_descriptor] ; Load GDT
  ret

disk_err:
  mov bx, msg_disk_err
  call rm_print
  jmp $

msg_load: db 'Loading kernel into memory. ',0
msg_disk_err: db 'Disk read error!',0
msg_rm: db 'Starting in 16-bit Real Mode. ',0
boot_drive: db 0
gdt_start:
gdt_null:       ; Manditory null descriptor
  dd 0x0        
  dd 0x0
gdt_code:       ; Code segment descriptor
  ; Base        <=> 0xC0000000    
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
gdt_data: ; Data segment descriptor
  ; Same as code segment descriptor except for the type flags:
  ; Type flags  <=> 0010b   <=> 0(code) 0(expand down) 1(writable) 0(accessed)
  dw 0xffff     
  dw 0x0        
  db 0x0        
  db 10010010b  
  db 11001111b  
  db 0x0       
gdt_end: 
gdt_descriptor: ; GDT size (bits 0-15), GDT address (bits 16-47)
  dw gdt_end - gdt_start - 1
  dd gdt_start
