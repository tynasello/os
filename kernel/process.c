/*
---------------------

Round-robin Scheduling

Each process has their own page directory, and each thread has their own stack
(however each thread shares the kernel head for now).

---------------------
*/

#include "include/process.h"
#include "include/idt.h"
#include "include/screen.h"
#include "include/vmm.h"
#include <stddef.h>

#define STACK_SIZE (4096 * 4)

uint8_t next_pid = 0;
uint8_t next_tid = 0;

typedef struct {
  Process *head_process;
  Process *curr_running_process;
} Scheduler;

Scheduler scheduler = {NULL, NULL};

Process *create_process() {
  __asm__ __volatile__("cli");
  Process *process = (Process *)kmalloc(sizeof(Process));
  process->pid = next_pid++;
  process->pd = create_process_pd();
  process->next = NULL;
  process->head_thread = NULL;
  process->curr_running_thread = NULL;
  if (scheduler.head_process == NULL) {
    scheduler.head_process = process;
  } else {
    Process *tail_process = scheduler.head_process;
    while (tail_process->next != NULL) {
      tail_process = tail_process->next;
    }
    tail_process->next = process;
  }
  __asm__ __volatile__("sti");
  return process;
}

Thread *create_thread(Process *process, void (*function)(uintptr_t),
                      uintptr_t arg) {
  __asm__ __volatile__("cli");
  Thread *thread = (Thread *)kmalloc(sizeof(Thread));
  thread->tid = next_tid++;
  thread->status = READY;
  thread->process = process;
  thread->next = NULL;
  thread->k_stack = kmalloc(STACK_SIZE);
  uintptr_t new_stack = (uintptr_t)thread->k_stack + STACK_SIZE;
  thread->context =
      (CpuContext *)(new_stack - sizeof(CpuContext) -
                     8); // - 8 (store return addr of 0 and arg at top of stack)
  thread->context->edi = 0;
  thread->context->esi = 0;
  thread->context->ebp = new_stack;
  thread->context->esp = (uintptr_t)&thread->context->int_no;
  thread->context->ebx = 0;
  thread->context->edx = 0;
  thread->context->ecx = 0;
  thread->context->eax = 0;
  thread->context->int_no = 32;
  thread->context->err_code = 0;
  thread->context->eip = (uintptr_t)function;
  thread->context->cs = 0x8;
  thread->context->eflags = 0x202;
  *(uintptr_t *)((uintptr_t)thread->context + sizeof(CpuContext)) = 0;
  *(uintptr_t *)((uintptr_t)thread->context + sizeof(CpuContext) + 4) = arg;
  if (process->head_thread == NULL) {
    process->head_thread = thread;
  } else {
    Thread *tail_thread = process->head_thread;
    while (tail_thread->next != NULL) {
      tail_thread = tail_thread->next;
    }
    tail_thread->next = thread;
  }
  __asm__ __volatile__("sti");
  return thread;
}

extern ProcessPd *process_pds;

void delete_process() {
  if (scheduler.head_process == scheduler.curr_running_process) {
    scheduler.head_process = scheduler.curr_running_process->next;
  } else {
    Process *prev = scheduler.head_process;
    while (prev->next != scheduler.curr_running_process) {
      prev = prev->next;
    }
    prev->next = scheduler.curr_running_process->next;
  }
  load_pd(process_pds->pd_pa);
  delete_process_pd(scheduler.curr_running_process->pd);
  kfree(scheduler.curr_running_process);
  scheduler.curr_running_process = NULL;
}

void *stack_to_delete = NULL;

void delete_thread() {
  if (scheduler.curr_running_process->head_thread ==
      scheduler.curr_running_process->curr_running_thread) {
    scheduler.curr_running_process->head_thread =
        scheduler.curr_running_process->curr_running_thread->next;
  } else {
    Thread *prev = scheduler.curr_running_process->head_thread;
    while (prev->next != scheduler.curr_running_process->curr_running_thread) {
      prev = prev->next;
    }
    prev->next = scheduler.curr_running_process->curr_running_thread->next;
  }
  stack_to_delete =
      (void *)scheduler.curr_running_process->curr_running_thread->k_stack;
  scheduler.curr_running_process->curr_running_thread = NULL;

  if (scheduler.curr_running_process->head_thread == NULL) {
    delete_process();
  }
}

extern void swtch(CpuContext *new);

void schedule(CpuContext *context) {
  if (stack_to_delete != NULL) {
    kfree(stack_to_delete);
    stack_to_delete = NULL;
  }

  if (scheduler.head_process == NULL) {
    return;
  }

  if (scheduler.curr_running_process != NULL &&
      scheduler.curr_running_process->head_thread == NULL) {
    delete_process();
  }
  if (scheduler.curr_running_process != NULL &&
      scheduler.curr_running_process->curr_running_thread != NULL) {
    if (scheduler.curr_running_process->curr_running_thread->status == DEAD) {
      delete_thread();
    } else {
      scheduler.curr_running_process->curr_running_thread->context = context;
    }
  }

  Process *next_process = scheduler.curr_running_process == NULL ||
                                  scheduler.curr_running_process->next == NULL
                              ? scheduler.head_process
                              : scheduler.curr_running_process->next;
  if (next_process == NULL) {
    return;
  }
  if (next_process != scheduler.curr_running_process) {
    load_pd(next_process->pd->pd_pa);
  }

  Thread *next_thread = next_process->curr_running_thread == NULL ||
                                next_process->curr_running_thread->next == NULL
                            ? next_process->head_thread
                            : next_process->curr_running_thread->next;
  if (next_thread == NULL) {
    return;
  }

  scheduler.curr_running_process = next_process;
  scheduler.curr_running_process->curr_running_thread = next_thread;
  swtch(next_thread->context);
}

void thread_exit() {
  scheduler.curr_running_process->curr_running_thread->status = DEAD;
  while (1) {
  }
}

/*

Helper Methods

*/

void print_thread_status(ThreadStatus status) {
  switch (status) {
  case READY:
    print("READY");
    break;
  case RUNNING:
    print("RUNNING");
    break;
  case DEAD:
    print("DEAD");
    break;
  }
}

void print_thread(Thread *thread) {
  if (thread == NULL) {
    return;
  }
  print("thread: ");
  print_hex((uintptr_t)thread);
  print(", ");
  print("tid: ");
  print_int(thread->tid);
  print(", ");
  print("status: ");
  print_thread_status(thread->status);
  print(", ");
  print("process: ");
  print_hex((uintptr_t)thread->process);
  print(", ");
  print("next: ");
  print_hex((uintptr_t)thread->next);
  print(", ");
  print("cpu context: **");
  print_cpu_context(thread->context);
  print("**");
}

void print_process(Process *process) {
  if (process == NULL) {
    return;
  }
  print("process: ");
  print_hex((uintptr_t)process);
  print(", ");
  print("pid: ");
  print_int(process->pid);
  print(", ");
  print("pd: ");
  print_hex((uintptr_t)process->pd);
  print(", ");
  print("next: ");
  print_hex((uintptr_t)process->next);
  print(",\n");
  print("threads: ");
  Thread *curr_thread = process->head_thread;
  while (curr_thread != NULL) {
    print("{");
    print_thread(curr_thread);
    print("}, ");
    curr_thread = curr_thread->next;
  }
}
