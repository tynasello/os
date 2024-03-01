disk_load:
  push dx

  mov ah, 0x02      ; Select BIOS read sector function
  mov al, dh        ; Number of sectors to read
  mov ch, 0x00      ; Read from cylinder 0
  mov dh, 0x00      ; Read from head 0
  mov cl, 0x02      ; Read from the second sector (first sector is the boot sector)

  int 0x13          ; BIOS interrupt for disk access

  jc disk_error     ; A set carry flag indicates error

  pop dx
  cmp dh, al        ; al <=> number of sectors read; dh <=> number of sectors requested to be read
  jne disk_error 
  ret

disk_error:
  mov bx, DISK_ERROR_MSG
  call print_string
  jmp $

DISK_ERROR_MSG:
  db 'Disk read error!',0
