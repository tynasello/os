#include "include/kb.h"
#include "include/screen.h"
#include "include/system.h"
#include "include/timer.h"
#include "include/vmm.h"

#define INIT_L 5 // Initial length of snake

typedef enum {
  UP,
  LEFT,
  DOWN,
  RIGHT,
} Dir;

typedef struct {
  int x;
  int y;
  VgaTextColor color;
} Food;

typedef struct Scale {
  int x;
  int y;
  VgaTextColor color;
  struct Scale *next;
} Scale;

typedef struct {
  Scale *head;
  Scale *tail;
  int len;
  Dir curr_dir;
} Snake;

typedef struct {
  int screen_w;
  int screen_h;
  int best_score;
  int curr_score;
  int running;
  int quitted;
  Snake s;
  Food f;
} Game;

static Game g;

static void tick();
static void user_in(char pressed);

/*

UI

*/

void draw_snake() {
  Scale *curr = g.s.head;
  for (int i = 0; i < g.s.len; i++) {
    print_square(curr->x, curr->y, curr->color);
    curr = curr->next;
  }
}

void draw_food() { 
  print_square(g.f.x, g.f.y, g.f.color); 
}

void draw_border(){
  print_block(0, 0, g.screen_w - 1, 0, WHITE);
}

void draw_best_score(){
  print_at("BEST: ", 2, 0, BLACK, WHITE);
  print_int_at(g.best_score, 7, 0, BLACK, WHITE);
}

void draw_score() { 
  print_int_at(g.curr_score, g.screen_w - 3, 0, BLACK, WHITE); 
}

/*

Game Setup

*/

static void init_game() {
  /* Initialize game structures */
  Scale *head = (Scale *)kmalloc(sizeof(Scale));
  Scale *tail;
  head->color = RED;
  head->x = 15;
  head->y = 15;

  Scale *curr = head;
  for (int i = 0; i < INIT_L - 1; i++) {
    Scale *scale = (Scale *)kmalloc(sizeof(Scale));
    scale->color = GREEN;
    scale->x = curr->x;
    scale->y = curr->y - 1;
    curr->next = scale;
    curr = curr->next;
    tail = scale;
  }

  Snake s = {
    .head = head, 
    .tail = tail, 
    .len = INIT_L, 
    .curr_dir = DOWN
  };

  Food f = {
    .x = g.screen_w / 2, 
    .y = g.screen_h / 2, 
    .color = WHITE 
  };

  g.s = s;
  g.f = f;
  g.curr_score = 0;

  /* Initialize display */
  clear_screen();
  draw_border();
  draw_best_score();
  draw_score();
  draw_snake();
  draw_food();
}

void snake_start() {
  disable_cursor();
  screen_backup();
  g.screen_w = get_screen_w();
  g.screen_h = get_screen_h();
  g.running = 0;
  g.quitted = 0;
  g.best_score = 0;
  g.curr_score = 0;
  init_game();
  register_timer_observer(&tick, 0);
  register_kb_observer(&user_in);
}

static void game_over() {
  g.running = 0;

  Scale *curr = g.s.head;
  Scale *next = 0x0;
  for (int i = 0; i < g.s.len; i++) {
    next = curr->next;
    kfree(curr);
    curr = next;
  }

  if (g.quitted) {
    deregister_timer_observer(&tick);
    deregister_kb_observer(&user_in);
    screen_restore();
    enable_cursor();
    print("Game Over");
    return;
  } 

  g.best_score = g.curr_score > g.best_score ? g.curr_score : g.best_score;
  init_game();

  return;
}

/*
 
Game Play

*/

static void user_in(char pressed) {
  if (pressed != 'w' && pressed != 'a' && pressed != 's' && pressed != 'd' &&
      pressed != 'g' && pressed != 'q') {
    return;
  }

  if (pressed == 'q') {
    g.quitted = 1;
    game_over();
    return;
  }

  if (!g.running && pressed != 'g') {
    return;
  }

  g.running = 1;
  g.s.curr_dir = pressed == 'w' ? UP
               : pressed == 'a' ? LEFT
               : pressed == 's' ? DOWN
               : pressed == 'd' ? RIGHT
               : g.s.curr_dir;
}

static int food_is_blocked() {
  Scale *curr = g.s.head;
  for (int i = 0; i < g.s.len; i++) {
    if (g.f.x == curr->x && g.f.y == curr->y) {
      return 1;
    }
    curr = curr->next;
  }
  return 0;
}

static void move() {
  /* Remove display of tail */
  int prev_tail_x = g.s.tail->x;
  int prev_tail_y = g.s.tail->y;
  print_square(prev_tail_x, prev_tail_y, BLACK);

  /* Move body */
  Scale *curr = g.s.head;
  int prev_x = curr->x;
  int prev_y = curr->y;
  for (int i = 0; i < g.s.len - 1; i++) {
    int curr_x = curr->next->x;
    int curr_y = curr->next->y;
    curr->next->x = prev_x;
    curr->next->y = prev_y;
    prev_x = curr_x;
    prev_y = curr_y;
    curr = curr->next;
  }

  /* Move head */
  switch (g.s.curr_dir){
    case UP:
      g.s.head->y--;
      break;
    case LEFT:
      g.s.head->x--;
      break;
    case DOWN:
      g.s.head->y++;
      break;
    case RIGHT:
      g.s.head->x++;
      break;
  }

  /* Check for border collision */
  if (g.s.head->x < 0 || g.s.head->x >= g.screen_w ||
      g.s.head->y < 1 || g.s.head->y >= g.screen_h) {
    game_over();
    return;
  }

  /* Check for self collision */
  curr = g.s.head->next;
  for (int i = 0; i < g.s.len - 1; i++) {
    if (g.s.head->x == curr->x && g.s.head->y == curr->y) {
      game_over();
      return;
    }
    curr = curr->next;
  }

  /* Grow if snake ate */
  if (g.s.head->x == g.f.x && g.s.head->y == g.f.y) {
    g.curr_score++;
    draw_score();

    Scale *growth = (Scale *)kmalloc(sizeof(Scale));
    growth->x = prev_tail_x;
    growth->y = prev_tail_y;
    growth->color = g.s.tail->color;
    g.s.tail->next = growth;
    g.s.tail = growth;
    g.s.len++;

    do {
      g.f.x = rand_range(0, g.screen_w - 1);
      g.f.y = rand_range(1, g.screen_h - 1);
    } while (food_is_blocked());
    draw_food();
  }

  /* Display updated snake position */
  draw_snake();
}

static void tick() {
  if (!g.running) {
    return;
  }
  move();
}
