#include <stdio.h>

template <typename T>
struct A{
  T a;
  
  int foo(){
    return a.f();
  };
};

struct B{
  int f(){
    printf("HOGE\n");
    return 0;
  };
};


int main()
{
  A<B> a;
  a.foo();
  return 0;
}
