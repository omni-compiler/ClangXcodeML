#include <stdio.h>

enum COLOR {
  RED = 100,
  YELLOW,
  BLUE,
};

void
print_color(COLOR c) {
  switch (c) {
  case RED: printf("RED\n"); break;
  case YELLOW: printf("YELLOW\n"); break;
  case BLUE: printf("BLUE\n"); break;
  }
}

int
main() {
  print_color(RED);
  print_color(YELLOW);
  print_color(BLUE);
}
