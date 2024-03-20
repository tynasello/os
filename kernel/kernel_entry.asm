[bits 32]

global idt_load     ; Function is declared in idt.c as "extern void idt_load();"
global isr0         ; ISR symbols used in isr.c
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

extern idtp
extern isr_fault_handler    ; Generic fault handler written in isr.c
extern irq_handler          ; Generic irq handler written in irq.c

;----------------------
;
; Kernel entry point
;
;----------------------
section .text

_start:
    extern kmain    ; Declare the entry point that exists in kernel.c
    call kmain 
    jmp $

;----------------------
;
; Interrupt Descriptor Table
;
;----------------------

idt_load:                   ; Load the IDT defined in 'idtp' and initialized in idt.c
    lidt [idtp]
    ret

isr0:                       ; Division Error
    cli                     ; Disable interrupts to prevent IRQs from firing while servicing another interrupt.
                            ; Otherwise conflicts could arise in kernel.
    push byte 0             ; Push dummy error code to maintain uniform stack frame. This is not always needed as
                            ; the processor will automatically push an error code for relevant interrupts.
    push byte 0             ; Push interrupt number
    jmp isr_fault_wrapper   ; Jump to generic ISR wrapper
isr1:                       ; Debug Exception
    cli
    push byte 0
    push byte 1
    jmp isr_fault_wrapper
isr2:                       ; Non-maskable Interrupt
    cli
    push byte 0
    push byte 2
    jmp isr_fault_wrapper
isr3:                       ; Breakpoint Exception
    cli
    push byte 0
    push byte 3
    jmp isr_fault_wrapper
isr4:                       ; Overflow Exception
    cli
    push byte 0
    push byte 4
    jmp isr_fault_wrapper
isr5:                       ; Bound Range Exceeded Exception
    cli
    push byte 0
    push byte 5
    jmp isr_fault_wrapper
isr6:                       ; Invalid Opcode Exception
    cli
    push byte 0
    push byte 6
    jmp isr_fault_wrapper
isr7:                       ; Device Not Available Exception
    cli
    push byte 0
    push byte 7
    jmp isr_fault_wrapper
isr8:                       ; Double Fault Exception
    cli
    push byte 8
    jmp isr_fault_wrapper
isr9:                       ; Coprocessor Segment Overrun Exception
    cli
    push byte 0
    push byte 9
    jmp isr_fault_wrapper
isr10:                      ; Invalid TSS Exception
    cli
    push byte 10
    jmp isr_fault_wrapper
isr11:                      ; Segment Not Present Exception
    cli
    push byte 11
    jmp isr_fault_wrapper
isr12:                      ; Stack-Segment Fault Exception
    cli
    push byte 12
    jmp isr_fault_wrapper
isr13:                      ; General Protection Exception
    cli
    push byte 13
    jmp isr_fault_wrapper
isr14:                      ; Page Fault Exception
    cli
    push byte 14
    jmp isr_fault_wrapper
isr15:                      ; Reserved Exception
    cli
    push byte 0
    push byte 15
    jmp isr_fault_wrapper
isr16:                      ; x87 Floating-Point Exception
    cli
    push byte 0
    push byte 16
    jmp isr_fault_wrapper
isr17:                      ; Alignment Check Exception
    cli
    push byte 17
    jmp isr_fault_wrapper
isr18:                      ; Machine Check Exception
    cli
    push byte 0
    push byte 18
    jmp isr_fault_wrapper
isr19:                      ; SIMD Floating-Point Exception
    cli
    push byte 0
    push byte 19
    jmp isr_fault_wrapper
isr20:                      ; Virtualization Exception
    cli
    push byte 0
    push byte 20
    jmp isr_fault_wrapper
isr21:                      ; Control Protection Exception
    cli
    push byte 21
    jmp isr_fault_wrapper
isr22:                      ; Reserved Exception
    cli
    push byte 0
    push byte 22
    jmp isr_fault_wrapper
isr23:                      ; Reserved Exception
    cli
    push byte 0
    push byte 23
    jmp isr_fault_wrapper
isr24:                      ; Reserved Exception
    cli
    push byte 0
    push byte 24
    jmp isr_fault_wrapper
isr25:                      ; Reserved Exception
    cli
    push byte 0
    push byte 25
    jmp isr_fault_wrapper
isr26:                      ; Reserved Exception
    cli
    push byte 0
    push byte 26
    jmp isr_fault_wrapper
isr27:                      ; Reserved Exception
    cli
    push byte 0
    push byte 27
    jmp isr_fault_wrapper
isr28:                      ; Reserved Exception
    cli
    push byte 0
    push byte 28
    jmp isr_fault_wrapper
isr29:                      ; Hypervisor Injection Exception
    cli
    push byte 29
    jmp isr_fault_wrapper
isr30:                      ; VMM Communication Exception
    cli
    push byte 30
    jmp isr_fault_wrapper
isr31:                      ; Security Exception
    cli
    push byte 0
    push byte 31
    jmp isr_fault_wrapper

;----------------------
;
; IRQ based ISRs used in irq.c
;
;----------------------

irq0:                   ; Timer
    cli                 ; Disable interrups just like ISRs 0-31
    push byte 0         ; IRQ don't generate an error.
                        ; So just like with many of ISRs 0-31, push a dummy error code to maintain uniform stack frame.
    push byte 32        ; Push Interrupt Vector number
    jmp irq_wrapper     ; Jump to generic IRQ wrapper 
irq1:                   ; Keyboard
    cli
    push byte 0
    push byte 33
    jmp irq_wrapper
irq2:                   ; Cascade for IRQs 8-15
    cli
    push byte 0
    push byte 34
    jmp irq_wrapper
irq3:                   ; COM2/COM4
    cli
    push byte 0
    push byte 35
    jmp irq_wrapper
irq4:                   ; COM1/COM3
    cli
    push byte 0
    push byte 36
    jmp irq_wrapper
irq5:                   ; LPT2
    cli
    push byte 0
    push byte 37
    jmp irq_wrapper
irq6:                   ; Floppy Disk
    cli
    push byte 0
    push byte 38
    jmp irq_wrapper
irq7:                   ; LPT1
    cli
    push byte 0
    push byte 39
    jmp irq_wrapper
irq8:                   ; CMOS Real-time Clock
    cli
    push byte 0
    push byte 40
    jmp irq_wrapper
irq9:                   ; Free for peripherals / Open interrupt
    cli
    push byte 0
    push byte 41
    jmp irq_wrapper
irq10:                  ; Free for peripherals / Open interrupt
    cli
    push byte 0
    push byte 42
    jmp irq_wrapper
irq11:                  ; Free for peripherals / Open interrupt
    cli
    push byte 0
    push byte 43
    jmp irq_wrapper
irq12:                  ; PS/2 Mouse
    cli
    push byte 0
    push byte 44
    jmp irq_wrapper
irq13:                  ; FPU / Coprocessor / Inter-processor
    cli
    push byte 0
    push byte 45
    jmp irq_wrapper
irq14:                  ; Primary ATA Hard Disk
    cli
    push byte 0
    push byte 46
    jmp irq_wrapper
irq15:                  ; Secondary ATA Hard Disk
    cli
    push byte 0
    push byte 47
    jmp irq_wrapper


;----------------------
;
; Generic ISR fault wrapper performs the following:
; 1) saves processor state (push all registers, including segment registers)
; 2) loads Kernel Data Segment descriptor (currently the only data segment) into data segment registers
; 3) push the stack ptr onto the stack, giving the fault handler access to the previous stack frame
; 4) call the fault handler defined in isr.c
; 4) restore the previous stack frame
;
;----------------------

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

;----------------------
;
; Generic IRQ wrapper for IRQ based ISRs.
; Uses same strategy as isr_fault_wrapper, but calls the generic IRQ handler instead of the ISR fault handler
;
;----------------------

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
