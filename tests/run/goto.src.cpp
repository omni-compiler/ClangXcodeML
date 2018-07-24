#include <stdio.h>

void
test_multiple_labels() {
  int i = 0;
label0:
label1:
label2:
label3:
  if (i > 9) {
    return;
  }
  ++i;
  switch (i % 4) {
    case 0:
      printf("zero\n");
      goto label0;
    case 1:
      printf("one\n");
      goto label1;
    case 2:
      printf("two\n");
      goto label2;
    default:
      printf("other\n");
      goto label3;
  }
}

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
  test_multiple_labels();
  return 0;
}
