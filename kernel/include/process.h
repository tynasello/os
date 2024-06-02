#ifndef __PROCESS_H
#define __PROCESS_H

#include "idt.h"
#include "vmm.h"
#include <stdint.h>

typedef enum { READY, RUNNING, DEAD } ThreadStatus;

typedef struct Process Process;

typedef struct Thread {
  uint8_t tid;
  ThreadStatus status;
  Process *process;
  struct Thread *next;
  CpuContext *context;
  uintptr_t k_stack;
} Thread;

/* A Process Control Block (PCB) */
typedef struct Process {
  uint8_t pid;
  ProcessPd *pd;
  struct Process *next;
  Thread *head_thread;
  Thread *curr_running_thread;
} Process;

void schedule(CpuContext *context);
Process *create_process();
Thread *create_thread(Process *process, void (*function)(uintptr_t),
                      uintptr_t arg);
void thread_exit();

void print_process(Process *process);
void print_thread(Thread *thread);

#endif
