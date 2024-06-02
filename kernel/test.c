#include "include/process.h"
#include "include/screen.h"
#include "include/snake.h"
#include "include/timer.h"

void t_one(int *a) {
  timer_wait(0.1);
  print_int(*a);
  print("\n");
  thread_exit();
}

void t_one_two() {
  timer_wait(2);
  print("1-2\n");
  thread_exit();
}

void t_two() {
  timer_wait(0.1);
  print("2\n");
  thread_exit();
}

void t_three() {
  timer_wait(1);
  print("3\n");
  thread_exit();
}

void t_three_two() {
  timer_wait(0.1);
  print("3-2\n");
  thread_exit();
}
void test_scheduling() {

  Process *p_one = create_process();
  Process *p_two = create_process();
  Process *p_three = create_process();

  int a = 1;
  create_thread(p_one, (void (*)(uintptr_t))t_one, (uintptr_t)&a);
  create_thread(p_one, t_one_two, 0);
  create_thread(p_two, t_two, 0);
  create_thread(p_three, t_three, 0);
  create_thread(p_three, t_three_two, 0);
}

void test_vm() { snake_start(); }
