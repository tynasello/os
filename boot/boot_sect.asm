[org 0x7c00] ; Provide base address (origin) of this file to the assembler
KERNEL_OFFSET equ 0x1000 ; The memory offset where the kernel will be loaded 

	mov [BOOT_DRIVE], dl 			; BIOS detects available storage devices
														; The location of the boot drive is stored in dl, hold on to this...
	mov bp, 0x9000 						; Setup stack
	mov sp, bp

	mov bx, MSG_REAL_MODE
	call print_string

	call load_kernel
	
	call switch_to_pm

	jmp $

%include "print_string.asm"
%include "print_hex.asm"
%include "gdt.asm"
%include "print_string_pm.asm"
%include "switch_to_pm.asm"
%include "disk_load.asm"

[bits 16]

load_kernel:
	mov bx, MSG_LOAD_KERNEL
	call print_string

	mov bx, KERNEL_OFFSET ; Load 15 sectors (512B * 15 = 7680B <=> 7.5KiB) to 0x1000 (ES:KERNEL_OFFSET <=> 0x0000:0x1000)
	mov dh, 15
	mov dl, [BOOT_DRIVE]
	call disk_load

	ret

[bits 32] ; This directive tells the assembler to encode into 32-bit instructions from now on

BEGIN_PM:
	call KERNEL_OFFSET

	jmp $

; Global variables
BOOT_DRIVE: db 0

MSG_REAL_MODE: db 'Starting in 16-bit Real Mode. ',0
MSG_LOAD_KERNEL: db 'Loading kernel into memory. ',0

; Padding and magic number for BIOS.
times 510-($-$$) db 0
dw 0xaa55
