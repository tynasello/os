[bits 32]

; Print a string starting at value stored in bx register.
; *Useful BIOS routines are no longer available in PM.*
; Must interact with VGA hardware explicitly.

VIDEO_MEMORY equ 0xb8000 ; VGA hardware is memory mapped to this address
WHITE_ON_BLACK equ 0x0f

print_string_pm:
	pusha
	mov edx, VIDEO_MEMORY
	add edx, 80*2*24 ; Print on last line for now (no cursor movement implemented)
 
print_string_pm_loop:
	mov al, [ebx]           ; al <=> ascii char to print (8 bits)
	mov ah, WHITE_ON_BLACK  ; ah <=> char attributes (foreground, background) (8 bits)

	cmp al, 0
	je print_string_pm_done

	mov [edx], ax           ; Write to VGA memory 

	add edx, 2              ; Move to next cell in VGA memory
	add ebx, 1              ; Move to next char in string to print

	jmp print_string_pm_loop

print_string_pm_done:
	popa
	ret
