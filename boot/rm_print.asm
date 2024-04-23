;----------
; Print a string starting at value stored in bx register

rm_print:
    mov ah, 0x0e ; Set "Teletype Output" BIOS operation for video BIOS services; prints character in al to screen

rm_print_loop:
    mov al, [bx]      
    cmp al, 0       ; 0 is the chosen null terminating byte 
    je  rm_print_done    
    int 0x10        ; BIOS interrupt for video services

    add bx, 0x1       
    jmp rm_print_loop   

rm_print_done:
    ret

;----------
; HEX - Print the hex value in bx register

; rm_print_hex:
;   ; Manipulate chars at hex_out to reflect bx
;   ; Handle fourth (MS) nibble
;   mov cx, bx
;   and cx, 0xf000 
;   shr cx, 12
;   call hex_to_ascii
;   mov [hex_out+2],cx 
;   ; Handle third nibble
;   mov cx, bx
;   and cx, 0x0f00
;   shr cx, 8
;   call hex_to_ascii
;   mov [hex_out+3],cx 
;   ; Handle second nibble
;   mov cx, bx
;   and cx, 0x00f0
;   shr cx, 4
;   call hex_to_ascii
;   mov [hex_out+4],cx 
;   ; Handle first (LS) nibble
;   mov cx, bx
;   and cx, 0x000f
;   call hex_to_ascii
;   mov [hex_out+5],cx 
;   ; hex_out now holds ascii for inputted hex
;   mov bx, hex_out
;   call rm_print
;   ret
;
; hex_to_ascii:
;   cmp cx, 0xa
;   jge hex_char_to_ascii
;   add cx, '0'
;   ret
;
; hex_char_to_ascii:
;   sub cx, 0xa
;   add cx, 'a'
;   ret
;
; hex_out: db '0x0000',
