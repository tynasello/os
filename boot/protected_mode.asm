[bits 32]

KERNEL_START_VA equ 0xC0000000
NUM_KERNEL_PTE equ 256  ; Kernel code loaded < 1MiB

NUM_PDE equ 1024
PAGE_FRAME_SIZE equ 4096
PD_PA equ 0x7E000 ; Memory location of the PD and PT (which maps kernel code)
KERNEL_PT_PA equ 0x7F000
TEMP_BOOT_PT_PA equ 0x100000 ; Used to enable execution of boot sector code after enabling paging

start_pm:
	;   Update all segment registers to use the data segment defined in GDT
	mov ax, DATA_SEG_SEL
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ebp, 0x90000; Update stack pointers (to near top of free space in memory)
	mov esp, ebp

init_pd:
	;   Initialize empty PD
	mov edi, PD_PA
	mov ecx, NUM_PDE
	xor eax, eax
	rep stosd; Store 32-bit value in eax (0) into edi (PD_PA) and incriment edi by 32-bits, ecx (1024) times

init_temp_kernel_pt:
	;    Once paging is enabled, the rest of the boot sector code still needs to be (identity) mapped.
	;    Use a temporary PT for this. Once the kernel code is executed, this temporary PT can be removed.
	mov  edi, TEMP_BOOT_PT_PA
	call create_kernel_pt
	add  ecx, 0; First PDE
	mov  eax, TEMP_BOOT_PT_PA
	call create_pde

init_kernel_pt:
	mov  edi, KERNEL_PT_PA
	call create_kernel_pt
	mov  ecx, 3072; PDE that maps VA of kernel code
	mov  eax, KERNEL_PT_PA
	call create_pde

enable_paging:
	;    Add recursive PDE
	mov  ecx, 4092; Last PDE
	mov  eax, PD_PA
	call create_pde
	;    Load PD
	mov  eax, PD_PA
	mov  cr3, eax
	;    Enable paging
	mov  eax, cr0
	or   eax, 0x80000000
	mov  cr0, eax

remap_gdt:
	;    GDT registers and table values deal in VAs. These values need modification once paging is enabled.
	mov  eax, gdt_descriptor
	mov  byte [eax + 5], 0xC0; Update GDT location
	add  eax, KERNEL_START_VA; Updated GDT descriptor location
	lgdt [eax]

update_stack:
	mov eax, esp
	add eax, KERNEL_START_VA
	mov ebp, eax
	mov esp, eax

jump_to_kernel_code:
	mov  eax, KERNEL_START_VA
	add  eax, KERNEL_OFFSET
	call eax
	jmp  $

create_pde:
	;   Create PDE at index ecx, mapping to page table located at eax
	mov edi, PD_PA
	add edi, ecx
	or  dword eax, 0x1; Mark as present
	mov [edi], eax
	ret

create_kernel_pt:
	;   Create a PT located at edi, mapping all kernel code (0x0 - 1MiB)
	mov ebx, 0
	mov ecx, NUM_KERNEL_PTE

fill_kernel_pt_loop:
	mov dword [edi], ebx; Map page frame to current PDE
	or  dword [edi], 0x1; Mark as present
	add ebx, PAGE_FRAME_SIZE; Move to next page frame
	add edi, 4; Move to next PDE
	sub ecx, 1
	jnz fill_kernel_pt_loop; Jump if ecx not 0
	ret
