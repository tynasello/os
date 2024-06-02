%include "rm_print.asm"

KERNEL_OFFSET equ 0x500 ; PA where kernel will be loaded
CODE_SEG_SEL equ gdt_code_seg_desc - gdt_start ; Define offsets (indexes) into GDT that will be used when setting segment registers
DATA_SEG_SEL equ gdt_data_seg_desc - gdt_start ; 0x0 -> NULL; 0x08 -> CODE; 0x10 -> DATA

start_rm:
	mov  [boot_drive], dl; BIOS detects available storage devices. The location of the boot drive is stored in dl.
	mov  bx, msg_rm
	call rm_print

load_kernel:
	;    Load kernel into RAM from boot drive
	mov  bx, msg_load
	call rm_print
	mov  bx, KERNEL_OFFSET; Where in RAM to load
	mov  dh, 40; Number of sectors to load (512B each)
	mov  dl, [boot_drive]; Location of boot drive

	push dx
	mov  ah, 0x02; Select BIOS read sector function
	mov  al, dh; Number of sectors to read
	mov  ch, 0x00; Read from cylinder 0
	mov  dh, 0x00; Read from head 0
	mov  cl, 0x02; Read from the second sector (first sector is the boot sector)
	int  0x13; BIOS interrupt for disk access
	jc   disk_err; A set carry flag indicates error
	pop  dx
	cmp  dh, al; al stores number of sectors read
	jne  disk_err

prepare_pm:
	;    Disable interrupts (current setup won't work in PM) and load GDT
	cli
	lgdt [gdt_descriptor]
	ret

disk_err:
	mov  bx, msg_disk_err
	call rm_print
	jmp  $

msg_load: db 'Loading kernel into memory. ',0
msg_disk_err: db 'Disk read error!',0
msg_rm: db 'Starting in 16-bit Real Mode. ',0
boot_drive: db 0

gdt_start:
	; Check Intel developer manual for GDT descriptor/entry structures.
	; Segment registers will be set to cover the entire addressable memory space (paging will be used).

gdt_null_desc:
	dd 0x0
	dd 0x0

gdt_code_seg_desc:
	;  Covers 0 - 4 GiB
	dw 0xffff
	dw 0x0
	db 0x0
	db 10011010b
	db 11001111b
	db 0x0

gdt_data_seg_desc:
	;  Covers 0 - 4 GiB
	dw 0xffff
	dw 0x0
	db 0x0
	db 10010110b
	db 11001111b
	db 0x0

gdt_end:
gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start
