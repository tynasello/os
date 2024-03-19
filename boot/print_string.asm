[bits 16]

; Print a string starting at value stored in bx register
PRINT_STRING:
    mov ah, 0x0e ; Set "Teletype Output" BIOS operation for video BIOS services; prints character in al to screen

PRINT_STRING_LOOP:
    mov al, [bx]      
    cmp al, 0       ; 0 is the chosen null terminating byte 
    je  PRINT_STRING_DONE    
    int 0x10        ; BIOS interrupt for video services

    add bx, 0x1       
    jmp PRINT_STRING_LOOP   

PRINT_STRING_DONE:
    ret
