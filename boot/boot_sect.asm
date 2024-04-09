[bits 16]

org 0x7c00 	; Provide base address (origin) of this file to the assembler.
						; When the BIOS finds the boot sector, it is loaded into memory at segment 0, address 0x7c00.

jmp START 

%include "print_string.asm"
%include "print_hex.asm"
%include "gdt.asm"
%include "disk_load.asm"

START:
	mov [BOOT_DRIVE], dl 			; BIOS detects available storage devices
														; The location of the boot drive is stored in dl, hold on to this...
	mov bx, MSG_REAL_MODE
	call PRINT_STRING
	call LOAD_KERNEL
	call SWITCH_TO_PM
	jmp $


;----------------------
;
; Data section
;
;----------------------

KERNEL_OFFSET equ 0x500 ; The memory offset where the kernel will be loaded 
MSG_REAL_MODE: db 'Starting in 16-bit Real Mode. ',0
MSG_LOAD_KERNEL: db 'Loading kernel into memory. ',0

BOOT_DRIVE: db 0

;----------------------
;
; Real mode
;
;----------------------

LOAD_KERNEL:
	mov bx, MSG_LOAD_KERNEL
	call PRINT_STRING
	mov bx, KERNEL_OFFSET ; Load 40 sectors (512B * 40 = 20KiB) to 0x500 (ES:KERNEL_OFFSET)
	mov dh, 40
	mov dl, [BOOT_DRIVE]
	call DISK_LOAD
	ret


;----------------------
;
; Protected mode
;
;----------------------

[bits 32] ; This directive tells the assembler to encode into 32-bit instructions from now on

%include "switch_to_pm.asm"
%include "print_string_pm.asm"

BEGIN_PM:
	call KERNEL_OFFSET
	jmp $

;----------------------
;
; Padding and magic number for BIOS.
;
;----------------------

times 510-($-$$) db 0
dw 0xaa55
