#include "include/idt.h"
#include "include/screen.h"
#include "include/vmm.h"
#include <stddef.h>

typedef enum {
    READY,
    RUNNING,
    DEAD
} ProcessStatus;

typedef struct Process {
    ProcessStatus status;
    CpuContext* context;
    struct Process* next;
} Process;

Process* head = NULL;
Process* curr = NULL;

void idle_main() {
    print("Idling ...\n");
    while (1) {
        __asm__("cli");
        __asm__("hlt");
    }
}

void schedule(CpuContext* context) {
    if (curr == NULL) {
        curr = (Process*)kmalloc(sizeof(Process));
        head = curr;
        curr->next = NULL;
    }

    curr->context = context; // Should kmalloc, perform deep copy ?
    curr->context->eip = (uintptr_t)idle_main;
    curr->status = READY;

    while (1) {
        Process* prev = curr;
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            curr = head;
        }

        if (curr != NULL && curr->status == DEAD) {
            // delete
        } else {
            curr->status = RUNNING;
            context = curr->context;
            break;
        }
    }
}
