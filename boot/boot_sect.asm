	org 0x7c00; Provide base address (origin) of this file to the assembler.
	;   When the BIOS finds the boot sector, it is loaded into memory at segment 0, address 0x7c00.

_start:
	call start_rm

	;   Set the first bit of control register cr0 to switch to PM
	mov eax, cr0
	or  eax, 0x1
	mov cr0, eax
	;   A far jump (a jump to a different segment) forces the CPU to flush its pipeline, ensuring that no 16-bit instructions will continue to execute.
	;   This also implicitly loads the CS register.
	jmp CODE_SEG_SEL:start_pm
	jmp $

%include "real_mode.asm"
%include "protected_mode.asm"

padding:
	;     Padding and magic number for BIOS.
	times 510-($-$$) db 0
	dw    0xaa55
