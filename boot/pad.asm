[bits 16]

; Append this to the end of the kernel image to ensure we always have at least 15 sectors (including boot sector) to read. 
; Otherwise qemu will hang when trying to read
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
times 256 dw 0x0000
