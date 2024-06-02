rm_print:
	;   Print a string starting at value stored in bx register
	mov ah, 0x0e; Set "Teletype Output" operation for BIOS video services (will print character in al to screen)

rm_print_loop:
	mov al, [bx]
	cmp al, 0; 0 is the chosen null terminating byte
	je  rm_print_done
	int 0x10; BIOS video services interrupt
	add bx, 0x1
	jmp rm_print_loop

rm_print_done:
	ret
