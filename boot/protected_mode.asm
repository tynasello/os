[bits 32]

NUM_PDE equ 1024
NUM_KERNEL_PTE equ 256  ; Kernel code lives in real-mode address space (< 1MiB)
PAGE_FRAME_SIZE equ 4096

KERNEL_CODE_VA equ 0xC0000000 ; Kernel code will start at this VA (really 0xC0000500)
PD_PA equ 0x7E000  
PT_PA equ 0x7F000
TEMP_PT_PA equ 0x100000
VM_PRESENT equ 0x1

start_pm:
  mov ax, DATA_SEG      ; Old way (RM) of segmenting is meaningless in PM,
  mov ds, ax            ; update all segment registers to use the data segment defined in GDT.
  mov ss, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ebp, 0x90000      ; Update stack position (to near top of free space in memory)
  mov esp, ebp

init_pd:
	; Initialize empty PD
	mov edi, PD_PA
  mov ecx, NUM_PDE
  xor eax, eax           
  rep stosd         

;------------
; This allows access to boot sector code after paging has been enabled.
; Once the kernel code is executed, this temp PDE can be removed.

init_temp_kernel_pt:
  ; Create PT
  mov edi, TEMP_PT_PA
  call create_pt
  ; Add PT to PD
	add ecx, 0 		        ; First PDE
	mov eax, TEMP_PT_PA      
	call create_pde

init_kernel_pt:
  ; Create PT
  mov edi, PT_PA
  call create_pt
  ; Add PT to PD
  mov ecx, 3072         ; PDE mapping kernel code VA
	mov eax, PT_PA      
	call create_pde

enable_paging:
	; Add recursive PDE
  mov ecx, 4092         ; Last PDE
	mov eax, PD_PA      
	call create_pde
  ; Load PD
	mov eax, PD_PA
  mov cr3, eax 
  ; Enable paging
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

remap_gdt:
  ; GDT registers and table values deal in VAs.
  ; Must modify these values once paging is enabled.
  ; This is needed since kerenel will reside in higher half
  ; (identity-mapped kernel PT is temporary and will be removed).
  ; mov eax, gdt_code
  ; mov byte [eax + 6], 0xC0
  ; mov eax, gdt_data
  ; mov byte [eax + 6], 0xC0
  mov eax, gdt_descriptor
  mov byte [eax + 5], 0xC0
  mov eax, gdt_descriptor
  add eax, KERNEL_CODE_VA
  lgdt [eax]

update_stack:
	mov eax, esp
	add eax, KERNEL_CODE_VA
	mov esp, eax

jump_to_kernel_code:
	mov eax, KERNEL_CODE_VA
	add eax, KERNEL_OFFSET 
	call eax
	jmp $

create_pde:
  mov edi, PD_PA
  add edi, ecx
  or dword eax, VM_PRESENT
  mov [edi], eax
  ret

create_pt:
  mov ebx, 0          
  mov ecx, NUM_KERNEL_PTE

fill_pt_loop:
  mov dword [edi], ebx    
  or dword [edi], VM_PRESENT
  add ebx, PAGE_FRAME_SIZE  ; Move to next page frame
  add edi, 4                ; Move to next PDE
  sub ecx, 1
  jnz fill_pt_loop       
  ret
