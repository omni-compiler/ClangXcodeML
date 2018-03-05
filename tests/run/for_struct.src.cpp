#include <stdio.h>

int
main() {
  for (struct {
         void func() {
           printf("%d\n", num);
         }
         int num;
       } x = {0};
       x.num < 10;
       ++x.num) {
    x.func();
  }
  for (int i = 0; i < 10; ++i)
    struct ForBody {
      ForBody(int x) {
        printf("ForBody%d\n", x);
      }
    } body(i);
}
