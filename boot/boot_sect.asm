org 0x7c00 	; Provide base address (origin) of this file to the assembler.
						; When the BIOS finds the boot sector, it is loaded into memory at segment 0, address 0x7c00.

_start:
	call start_rm

  mov eax, cr0          ; Set the first bit of control register cr0 to switch to PM
  or eax, 0x1
  mov cr0, eax
  jmp CODE_SEG:start_pm ; A far jump (a jump to a different segment) forces the CPU to
                        ; Flush its pipeline, ensuring that no 16-bit instructions 
                        ; Will continue to execute. This also implicitly loads the CS register.
	jmp $

%include "real_mode.asm"
%include "protected_mode.asm"

padding:
  ; Padding and magic number for BIOS.
  times 510-($-$$) db 0
  dw 0xaa55
