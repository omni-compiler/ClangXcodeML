#include <stdio.h>

class ClassA {
public:
  ClassA(int i) {
    num = i;
  }
  friend int func(const ClassA &a);

private:
  int num;
};

int
func(const ClassA &a) {
  return a.num * a.num;
}

int
main() {
  ClassA obj(20);
  printf("%d\n", func(obj));
  return 0;
}
