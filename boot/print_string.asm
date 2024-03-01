; Print a string starting at value stored in bx register
print_string:
    mov ah, 0x0e ; Set "Teletype Output" BIOS operation for video BIOS services; prints character in al to screen

print_string_loop:
    mov al, [bx]      
    cmp al, 0       ; 0 is the chosen null terminating byte 
    je  print_string_done    
    int 0x10        ; BIOS interrupt for video services

    add bx, 0x1       
    jmp print_string_loop   

print_string_done:
    ret
