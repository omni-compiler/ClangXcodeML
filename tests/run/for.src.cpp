#include <stdio.h>

int
main() {
  int ans = 0;
  for (int i = 1; i <= 100; ++i) {
    ans += i;
  }
  printf("%d\n", ans);
  return 0;
}
