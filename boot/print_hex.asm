; Print the hex value in bx register
print_hex:
  ; Manipulate chars at HEX_OUT to reflect bx

  ; Handle fourth (MS) nibble
  mov cx, bx
  and cx, 0xf000 
  shr cx, 12
  call hex_to_ascii
  mov [HEX_OUT+2],cx 

  ; Handle third nibble
  mov cx, bx
  and cx, 0x0f00
  shr cx, 8
  call hex_to_ascii
  mov [HEX_OUT+3],cx 

  ; Handle second nibble
  mov cx, bx
  and cx, 0x00f0
  shr cx, 4
  call hex_to_ascii
  mov [HEX_OUT+4],cx 

  ; Handle first (LS) nibble
  mov cx, bx
  and cx, 0x000f
  call hex_to_ascii
  mov [HEX_OUT+5],cx 

  ; HEX_OUT now holds ascii for inputted hex
  mov bx, HEX_OUT
  call print_string
  ret

hex_to_ascii:
  cmp cx, 0xa
  jge hex_char_to_ascii
  add cx, '0'
  ret

hex_char_to_ascii:
  sub cx, 0xa
  add cx, 'a'
  ret

HEX_OUT:
  db '0x0000',
