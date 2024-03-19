[bits 16]

;----------------------
; Data section
;----------------------

HEX_OUT:
  db '0x0000',

;----------------------

; Print the hex value in bx register
PRINT_HEX:
  ; Manipulate chars at HEX_OUT to reflect bx

  ; Handle fourth (MS) nibble
  mov cx, bx
  and cx, 0xf000 
  shr cx, 12
  call HEX_TO_ASCII
  mov [HEX_OUT+2],cx 

  ; Handle third nibble
  mov cx, bx
  and cx, 0x0f00
  shr cx, 8
  call HEX_TO_ASCII
  mov [HEX_OUT+3],cx 

  ; Handle second nibble
  mov cx, bx
  and cx, 0x00f0
  shr cx, 4
  call HEX_TO_ASCII
  mov [HEX_OUT+4],cx 

  ; Handle first (LS) nibble
  mov cx, bx
  and cx, 0x000f
  call HEX_TO_ASCII
  mov [HEX_OUT+5],cx 

  ; HEX_OUT now holds ascii for inputted hex
  mov bx, HEX_OUT
  call PRINT_STRING
  ret

HEX_TO_ASCII:
  cmp cx, 0xa
  jge HEX_CHAR_TO_ASCII
  add cx, '0'
  ret

HEX_CHAR_TO_ASCII:
  sub cx, 0xa
  add cx, 'a'
  ret
