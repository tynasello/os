[bits 32]

; Declare the entry point that exists in kernel.c
[extern _start]
call _start 
jmp $

; Function is declared in idt.c as "extern void idt_load();"
global idt_load
extern idtp

; Load the IDT defined in 'idtp' and initialized in idt.c
idt_load:
    lidt [idtp]
    ret

; ISR symbols used in isr.c
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

; IRQ based ISR symbols used in irq.c
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

; Division Error
isr0:
    cli                     ; Disable interrupts to prevent IRQs from firing while servicing another interrupt. 
                            ; Otherwise conflicts could arise in kernel.
    push byte 0             ; Push dummy error code to maintain uniform stack frame. This is not always needed as
                            ; the processor will automatically push an error code for relevant interrupts.
    push byte 0             ; Pust interrupt number
    jmp isr_fault_wrapper   ; Jump to generic ISR wrapper

; Debug Exception
isr1:
    cli
    push byte 0
    push byte 1
    jmp isr_fault_wrapper

; Non-maskable Interrupt
isr2:
    cli
    push byte 0
    push byte 2
    jmp isr_fault_wrapper

; Breakpoint Exception
isr3:
    cli
    push byte 0
    push byte 3
    jmp isr_fault_wrapper

; Overflow Exception
isr4:
    cli
    push byte 0
    push byte 4
    jmp isr_fault_wrapper

; Bound Range Exceeded Exception
isr5:
    cli
    push byte 0
    push byte 5
    jmp isr_fault_wrapper

; Invalid Opcode Exception
isr6:
    cli
    push byte 0
    push byte 6
    jmp isr_fault_wrapper

; Device Not Available Exception
isr7:
    cli
    push byte 0
    push byte 7
    jmp isr_fault_wrapper

; Double Fault Exception
isr8:
    cli
    push byte 8
    jmp isr_fault_wrapper

; Coprocessor Segment Overrun Exception
isr9:
    cli
    push byte 0
    push byte 9
    jmp isr_fault_wrapper

; Invalid TSS Exception
isr10:
    cli
    push byte 10
    jmp isr_fault_wrapper

; Segment Not Present Exception
isr11:
    cli
    push byte 11
    jmp isr_fault_wrapper

; Stack-Segment Fault Exception
isr12:
    cli
    push byte 12
    jmp isr_fault_wrapper

; General Protection Exception
isr13:
    cli
    push byte 13
    jmp isr_fault_wrapper

; Page Fault Exception
isr14:
    cli
    push byte 14
    jmp isr_fault_wrapper

; Reserved Exception
isr15:
    cli
    push byte 0
    push byte 15
    jmp isr_fault_wrapper

; x87 Floating-Point Exception
isr16:
    cli
    push byte 0
    push byte 16
    jmp isr_fault_wrapper

; Alignment Check Exception
isr17:
    cli
    push byte 17
    jmp isr_fault_wrapper

; Machine Check Exception
isr18:
    cli
    push byte 0
    push byte 18
    jmp isr_fault_wrapper

; SIMD Floating-Point Exception
isr19:
    cli
    push byte 0
    push byte 19
    jmp isr_fault_wrapper

; Virtualization Exception
isr20:
    cli
    push byte 0
    push byte 20
    jmp isr_fault_wrapper

; Control Protection Exception
isr21:
    cli
    push byte 21
    jmp isr_fault_wrapper

; Reserved Exception
isr22:
    cli
    push byte 0
    push byte 22
    jmp isr_fault_wrapper

; Reserved Exception
isr23:
    cli
    push byte 0
    push byte 23
    jmp isr_fault_wrapper

; Reserved Exception
isr24:
    cli
    push byte 0
    push byte 24
    jmp isr_fault_wrapper

; Reserved Exception
isr25:
    cli
    push byte 0
    push byte 25
    jmp isr_fault_wrapper

; Reserved Exception
isr26:
    cli
    push byte 0
    push byte 26
    jmp isr_fault_wrapper

; Reserved Exception
isr27:
    cli
    push byte 0
    push byte 27
    jmp isr_fault_wrapper

; Reserved Exception
isr28:
    cli
    push byte 0
    push byte 28
    jmp isr_fault_wrapper

; Hypervisor Injection Exception
isr29:
    cli
    push byte 29
    jmp isr_fault_wrapper

; VMM Communication Exception
isr30:
    cli
    push byte 30
    jmp isr_fault_wrapper

; Security Exception
isr31:
    cli
    push byte 0
    push byte 31
    jmp isr_fault_wrapper

; Timer
irq0:
    cli                 ; Disable interrups just like ISRs 0-31
    push byte 0         ; IRQ don't generate an error.
                        ; So just like with many of ISRs 0-31, push a dummy error code to maintain uniform stack frame.
    push byte 32        ; Push Interrupt Vector number
    jmp irq_wrapper     ; Jump to generic IRQ wrapper 

; Keyboard
irq1:
    cli
    push byte 0
    push byte 33
    jmp irq_wrapper

; Cascade for IRQs 8-15
irq2:
    cli
    push byte 0
    push byte 34
    jmp irq_wrapper

; COM2/COM4
irq3:
    cli
    push byte 0
    push byte 35
    jmp irq_wrapper

; COM1/COM3
irq4:
    cli
    push byte 0
    push byte 36
    jmp irq_wrapper

; LPT2
irq5:
    cli
    push byte 0
    push byte 37
    jmp irq_wrapper

; Floppy Disk
irq6:
    cli
    push byte 0
    push byte 38
    jmp irq_wrapper

; LPT1
irq7:
    cli
    push byte 0
    push byte 39
    jmp irq_wrapper

; CMOS Real-time Clock
irq8:
    cli
    push byte 0
    push byte 40
    jmp irq_wrapper

; Free for peripherals / Open interrupt
irq9:
    cli
    push byte 0
    push byte 41
    jmp irq_wrapper

; Free for peripherals / Open interrupt
irq10:
    cli
    push byte 0
    push byte 42
    jmp irq_wrapper

; Free for peripherals / Open interrupt
irq11:
    cli
    push byte 0
    push byte 43
    jmp irq_wrapper

; PS/2 Mouse
irq12:
    cli
    push byte 0
    push byte 44
    jmp irq_wrapper

; FPU / Coprocessor / Inter-processor
irq13:
    cli
    push byte 0
    push byte 45
    jmp irq_wrapper

; Primary ATA Hard Disk
irq14:
    cli
    push byte 0
    push byte 46
    jmp irq_wrapper

; Secondary ATA Hard Disk
irq15:
    cli
    push byte 0
    push byte 47
    jmp irq_wrapper

; Generic fault handler written in isr.c
extern isr_fault_handler

; Generic irq handler written in irq.c
extern irq_handler

; Generic ISR fault wrapper performs the following:
; 1) saves processor state (push all registers, including segment registers)
; 2) loads Kernel Data Segment descriptor (currently the only data segment) into data segment registers
; 3) push the stack ptr onto the stack, giving the fault handler access to the previous stack frame
; 4) call the fault handler defined in isr.c
; 4) restore the previous stack frame
isr_fault_wrapper:
    pusha               ; Save processor state
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10        ; Load the Kernel Data Segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp        ; Push a ptr to the stack for the fault handler to use
    push eax
    mov eax, isr_fault_handler ; Call the generic fault handler in isr.c
    call eax       
    pop eax             ; Restore stack, popping registers in the reverse order that they were pushed
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8          ; Pop the pushed error code and ISR number.
                        ; The stack pointer is incrimented by 8 because each stack entry takes up 4 bytes in PM even if not needed.
                        ; Remember that the stack grows downwards. By incrimenting ESP, the stack shrinks.
    iret                ; Pop the following 5 things at once: EIP, CS, EFLAGS, ESP, and SS,
                        ; which are automatically pushed to the stack by the processor when an interrupt occurs. 
                        ; The CPU will load the segment-selector value from the associated IDT descriptor into CS.

; Generic IRQ wrapper for IRQ based ISRs.
; Uses same strategy as isr_fault_wrapper, but calls the generic IRQ handler instead of the ISR fault handler
irq_wrapper:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
