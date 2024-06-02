[bits 32]

%define MASTER_CMD_PORT 0x20
%define SLAVE_CMD_PORT  0xA0
%define EOI             0x20; End of interrupt (EOI) command used to notify PICs when interrupts have been serviced.

global idt_load
global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15
global irq_return

extern isr_fault_handler
extern irq_handler
extern port_byte_out

	;----------------------

	; Exception-based Interrupt Service Routines

	;----------------------

isr0:
	;    Division Error
	;    Disable interrupts to prevent IRQs from firing while servicing another interrupt.
	;    Otherwise problems could arise in kernel. This can probably be removed as a interrupt
	;    gate automatically disables the interrupt bit in the flags register.
	cli
	;    Push dummy error code to maintain uniform stack frame. This is not always needed as
	;    the processor will automatically push an error code for relevant interrupts.
	push byte 0
	push byte 0; Push interrupt vector number
	jmp  exception_wrapper; Jump to generic ISR wrapper

isr1:
	;    Debug Exception
	cli
	push byte 0
	push byte 1
	jmp  exception_wrapper

isr2:
	;    Non-maskable Interrupt
	cli
	push byte 0
	push byte 2
	jmp  exception_wrapper

isr3:
	;    Breakpoint Exception
	cli
	push byte 0
	push byte 3
	jmp  exception_wrapper

isr4:
	;    Overflow Exception
	cli
	push byte 0
	push byte 4
	jmp  exception_wrapper

isr5:
	;    Bound Range Exceeded Exception
	cli
	push byte 0
	push byte 5
	jmp  exception_wrapper

isr6:
	;    Invalid Opcode Exception
	cli
	push byte 0
	push byte 6
	jmp  exception_wrapper

isr7:
	;    Device Not Available Exception
	cli
	push byte 0
	push byte 7
	jmp  exception_wrapper

isr8:
	;    Double Fault Exception
	cli
	push byte 8
	jmp  exception_wrapper

isr9:
	;    Coprocessor Segment Overrun Exception
	cli
	push byte 0
	push byte 9
	jmp  exception_wrapper

isr10:
	;    Invalid TSS Exception
	cli
	push byte 10
	jmp  exception_wrapper

isr11:
	;    Segment Not Present Exception
	cli
	push byte 11
	jmp  exception_wrapper

isr12:
	;    Stack-Segment Fault Exception
	cli
	push byte 12
	jmp  exception_wrapper

isr13:
	;    General Protection Exception
	cli
	push byte 13
	jmp  exception_wrapper

isr14:
	;    Page Fault Exception
	cli
	push byte 14
	jmp  exception_wrapper

isr15:
	;    Reserved Exception
	cli
	push byte 0
	push byte 15
	jmp  exception_wrapper

isr16:
	;    x87 Floating-Point Exception
	cli
	push byte 0
	push byte 16
	jmp  exception_wrapper

isr17:
	;    Alignment Check Exception
	cli
	push byte 17
	jmp  exception_wrapper

isr18:
	;    Machine Check Exception
	cli
	push byte 0
	push byte 18
	jmp  exception_wrapper

isr19:
	;    SIMD Floating-Point Exception
	cli
	push byte 0
	push byte 19
	jmp  exception_wrapper

isr20:
	;    Virtualization Exception
	cli
	push byte 0
	push byte 20
	jmp  exception_wrapper

isr21:
	;    Control Protection Exception
	cli
	push byte 21
	jmp  exception_wrapper

isr22:
	;    Reserved Exception
	cli
	push byte 0
	push byte 22
	jmp  exception_wrapper

isr23:
	;    Reserved Exception
	cli
	push byte 0
	push byte 23
	jmp  exception_wrapper

isr24:
	;    Reserved Exception
	cli
	push byte 0
	push byte 24
	jmp  exception_wrapper

isr25:
	;    Reserved Exception
	cli
	push byte 0
	push byte 25
	jmp  exception_wrapper

isr26:
	;    Reserved Exception
	cli
	push byte 0
	push byte 26
	jmp  exception_wrapper

isr27:
	;    Reserved Exception
	cli
	push byte 0
	push byte 27
	jmp  exception_wrapper

isr28:
	;    Reserved Exception
	cli
	push byte 0
	push byte 28
	jmp  exception_wrapper

isr29:
	;    Hypervisor Injection Exception
	cli
	push byte 29
	jmp  exception_wrapper

isr30:
	;    VMM Communication Exception
	cli
	push byte 30
	jmp  exception_wrapper

isr31:
	;    Security Exception
	cli
	push byte 0
	push byte 31
	jmp  exception_wrapper

	;----------------------

	; IRQ-based ISRs

	;----------------------

irq0:
	;    Timer
	cli
	push byte 0; Push a dummy error code to maintain uniform stack frame
	push byte 32
	jmp  irq_wrapper

irq1:
	;    Keyboard
	cli
	push byte 0
	push byte 33
	jmp  irq_wrapper

irq2:
	;    Cascade for IRQs 8-15
	cli
	push byte 0
	push byte 34
	jmp  irq_wrapper

irq3:
	;    COM2/COM4
	cli
	push byte 0
	push byte 35
	jmp  irq_wrapper

irq4:
	;    COM1/COM3
	cli
	push byte 0
	push byte 36
	jmp  irq_wrapper

irq5:
	;    LPT2
	cli
	push byte 0
	push byte 37
	jmp  irq_wrapper

irq6:
	;    Floppy Disk
	cli
	push byte 0
	push byte 38
	jmp  irq_wrapper

irq7:
	;    LPT1
	cli
	push byte 0
	push byte 39
	jmp  irq_wrapper

irq8:
	;    CMOS Real-time Clock
	cli
	push byte 0
	push byte 40
	jmp  irq_wrapper

irq9:
	;    Free for peripherals / Open interrupt
	cli
	push byte 0
	push byte 41
	jmp  irq_wrapper

irq10:
	;    Free for peripherals / Open interrupt
	cli
	push byte 0
	push byte 42
	jmp  irq_wrapper

irq11:
	;    Free for peripherals / Open interrupt
	cli
	push byte 0
	push byte 43
	jmp  irq_wrapper

irq12:
	;    PS/2 Mouse
	cli
	push byte 0
	push byte 44
	jmp  irq_wrapper

irq13:
	;    FPU / Coprocessor / Inter-processor
	cli
	push byte 0
	push byte 45
	jmp  irq_wrapper

irq14:
	;    Primary ATA Hard Disk
	cli
	push byte 0
	push byte 46
	jmp  irq_wrapper

irq15:
	;    Secondary ATA Hard Disk
	cli
	push byte 0
	push byte 47
	jmp  irq_wrapper

	;----------------------

	; 1) Save processor state
	; 2) Load kernel DS
	; 3) Push the current stack ptr onto the stack, giving kernel access to the previous stack frame (CPU context pointer)
	; 4) Call the generic handler defined in C
	; 5) Restore the previous stack frame

	; When an interrupt occurs, the processor automatically pushes EIP, CS, EFLAGS
	; *ESP, and *SS when an interrupt occurs. The iret command pops these registers.
	; * - when handler has different privilege level than interrupted procedure.

	; The custom ISR wrapper pushes the following:
	; - Segment registers: gs, fs, es, ds
	; - General-purpose registes: edi, esi, ebp, esp, ebx, edx, ecx, eax
	; - int_no, err_code (for exceptions)

	;----------------------

	extern print_hex

exception_wrapper:
	pusha
	push esp
	call isr_fault_handler
	add  esp, 4; Start restoring stack
	popa
	add  esp, 8; Pop the pushed error code and interrupt vector number (both 4B long).
	iret

irq_wrapper:
	pusha
	push esp
	call irq_handler
	add  esp, 4

irq_return:
	popa

handle_interrupt:
	;    The CPU tells the PIC that the interrupt is complete by writing an EOI byte to
	;    the command port. In the case that the interrupt number is greater than 40
	;    (IRQ 8 or higher), then the slave must be notified as well as the master.
	cmp  [esp], dword 40
	jl   notify_master_only
	push SLAVE_CMD_PORT
	push EOI
	call port_byte_out
	add  esp, 8

notify_master_only:
	push MASTER_CMD_PORT
	push EOI
	call port_byte_out
	add  esp, 8

irq_int_return:
	add esp, 8
	iret
