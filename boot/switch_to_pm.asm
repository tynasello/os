[bits 16]

switch_to_pm:
  cli                   ; Disable interrupts for now (current set-up won't work in PM)

  lgdt [gdt_descriptor] ; Load GDT

  mov eax, cr0          ; Set the first bit of control register cr0 to switch to PM
  or eax, 0x1
  mov cr0, eax

  jmp CODE_SEG:init_pm  ; A far jump (a jump to a different segment) forces the CPU to
                        ; Flush its pipeline, ensuring that no 16-bit instructions 
                        ; Will continue to execute.
                        ; This also implicitly loads the CS register.

[bits 32]

init_pm:
  mov ax, DATA_SEG    ; Old way (RM) of segmenting is meaningless in PM,
  mov ds, ax          ; update all segment registers to use the data segment defined in GDT.
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov ebp, 0x90000    ; Update stack position (to near top of free space in memory)
  mov esp, ebp

  call BEGIN_PM   
