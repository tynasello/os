#include "include/kb.h"
#include "include/screen.h"
#include "include/system.h"
#include "include/timer.h"
#include "include/vmm.h"

#define INIT_L 5
#define MARGIN 2

typedef enum Dir {
  UP,
  LEFT,
  DOWN,
  RIGHT,
} Dir;

typedef struct Food {
  int x;
  int y;
  Color c;
} Food;

typedef struct Scale {
  int x;
  int y;
  Color c;
  struct Scale *next;
} Scale;

typedef struct Snake {
  Scale *head;
  Scale *tail;
  int l;
  Dir curr_dir;
} Snake;

typedef struct Game {
  int screen_w;
  int screen_h;
  int best_scr;
  int curr_scr;
  int started;
  int running;
  int quit;
  Snake s;
  Food f;
} Game;

static Game g;

static void clocked();
static void usr_in(char pressed);

void draw_snake() {
  Scale *curr = g.s.head;
  for (int i = 0; i < g.s.l; i++) {
    print_square(curr->x, curr->y, curr->c);
    curr = curr->next;
  }
}

void draw_food() { print_square(g.f.x, g.f.y, g.f.c); }

void draw_score() { print_int_at(g.curr_scr, 2, g.screen_w - 2, BLACKoWHITE); }

static void reset_game() {
  clear_screen();
  print_block(0, g.screen_w, MARGIN - 1, MARGIN - 1, BLACKoWHITE);
  print_block(0, g.screen_w, MARGIN, MARGIN, BLACKoWHITE);

  print_at("BEST: ", MARGIN, 0, BLACKoWHITE);
  print_int_at(g.best_scr, MARGIN, 5, BLACKoWHITE);

  draw_score();

  Scale *head = (Scale *)kmalloc(sizeof(Scale));
  head->c = REDoBLACK;
  head->x = 15;
  head->y = 15;

  Scale *curr = head;
  Scale *tail;

  for (int i = 0; i < INIT_L - 1; i++) {
    Scale *next = (Scale *)kmalloc(sizeof(Scale));
    tail = next;
    next->c = GREENoBLACK;
    next->x = curr->x;
    next->y = curr->y - 1;
    curr->next = next;
    curr = curr->next;
  }

  Snake s = {};
  s.head = head;
  s.tail = tail;
  s.l = INIT_L;
  s.curr_dir = DOWN;

  Food f = {g.screen_w / 2, g.screen_h / 2, BLACKoWHITE};

  g.s = s;
  g.f = f;
  draw_snake();
  draw_food();
}

void snake_strt() {
  screen_backup();

  g.screen_w = get_screen_w();
  g.screen_h = get_screen_h();
  g.started = 0;
  g.running = 0;
  g.quit = 0;
  g.best_scr = 0;
  g.curr_scr = 0;

  reset_game();

  register_timer_observer(&clocked, 0);
  register_kb_observer(&usr_in);
}

static void game_over() {
  g.running = 0;

  if (g.quit) {
    deregister_timer_observer(&clocked);
    deregister_kb_observer(&usr_in);
    screen_restore();
  } else {
    if (g.curr_scr > g.best_scr) {
      g.best_scr = g.curr_scr;
    }
    g.curr_scr = 0;
    reset_game();
    g.started = 0;
  }

  return;
}

static void usr_in(char pressed) {
  if (pressed != 'w' && pressed != 'a' && pressed != 's' && pressed != 'd' &&
      pressed != 'g' && pressed != 'r' && pressed != 'q') {
    return;
  }

  if (pressed == 'q') {
    g.quit = 1;
    game_over();
    return;
  }

  if (!g.started && pressed != 'g') {
    return;
  }

  if (pressed == 'g' && !g.running) {
    g.started = 1;
    g.running = 1;
  }

  if (!g.running) {
    return;
  }

  if (pressed == 'w') {
    g.s.curr_dir = UP;
  } else if (pressed == 'a') {
    g.s.curr_dir = LEFT;
  } else if (pressed == 's') {
    g.s.curr_dir = DOWN;
  } else if (pressed == 'd') {
    g.s.curr_dir = RIGHT;
  }
}

static int foodBlocked() {
  Scale *curr = g.s.head;
  for (int i = 0; i < g.s.l; i++) {
    if (g.f.x == curr->x && g.f.y == curr->y) {
      return 1;
    }
    curr = curr->next;
  }
  return 0;
}

static void move() {
  /* Remove tail */

  int init_tail_x = g.s.tail->x;
  int init_tail_y = g.s.tail->y;
  print_square(g.s.tail->x, g.s.tail->y, BLACKoBLACK);

  /* Move body */

  Scale *curr = g.s.head;

  int curr_x = curr->x;
  int curr_y = curr->y;

  for (int i = 0; i < g.s.l - 1; i++) {
    int tempx = curr->next->x;
    int tempy = curr->next->y;
    curr->next->x = curr_x;
    curr->next->y = curr_y;
    curr_x = tempx;
    curr_y = tempy;
    curr = curr->next;
  }

  /* Move head */

  if (g.s.curr_dir == UP) {
    g.s.head->y--;
  } else if (g.s.curr_dir == LEFT) {
    g.s.head->x--;
  } else if (g.s.curr_dir == DOWN) {
    g.s.head->y++;
  } else if (g.s.curr_dir == RIGHT) {
    g.s.head->x++;
  }

  /* Check for border collision */

  if (g.s.head->x < 0 || g.s.head->x >= g.screen_w ||
      g.s.head->y < MARGIN + 1 || g.s.head->y >= g.screen_h) {
    game_over();
    return;
  }

  /* Check for self collision */

  curr = g.s.head->next;
  for (int i = 0; i < g.s.l - 1; i++) {
    if (g.s.head->x == curr->x && g.s.head->y == curr->y) {
      game_over();
      return;
    }
    curr = curr->next;
  }

  /* Grow */

  if (g.s.head->x == g.f.x && g.s.head->y == g.f.y) {
    g.curr_scr++;
    draw_score();

    Scale *growth = (Scale *)kmalloc(sizeof(Scale));
    growth->x = init_tail_x;
    growth->y = init_tail_y;
    growth->c = g.s.tail->c;

    g.s.tail->next = growth;
    g.s.tail = growth;
    g.s.l++;

    do {
      g.f.x = rand_range(MARGIN, g.screen_w - MARGIN);
      g.f.y = rand_range(MARGIN, g.screen_h - MARGIN);
    } while (foodBlocked());

    draw_food();
  }

  /* Draw */

  draw_snake();
}

static void clocked() {
  if (!g.running) {
    return;
  }

  move();
}
