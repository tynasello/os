[bits 32]

global swtch

extern irq_return

swtch:
	mov esp, [esp+4]
	jmp irq_return
