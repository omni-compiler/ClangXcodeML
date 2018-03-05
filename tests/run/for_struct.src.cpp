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
}
