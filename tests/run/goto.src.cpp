#include <stdio.h>

int
main() {
  int n = 25;
cond:
  printf("%d\n", n);
  if (n == 1) {
    goto end;
  }
  if (n % 2) {
    n = n * 3 + 1;
    goto cond;
  } else {
    n /= 2;
    goto cond;
  }
end:
  printf("END\n");
  return 0;
}
