#include <stdio.h>
template <typename T, int x>
struct A
{
  T a[x];
  A()
  {
    for(int i=0; i < x; i++){
      a[i] = 0;
    }
  }
  T foo(int i)
  {
    return a[i];
  }

};

struct D : public A<int , 3>{
  using BASE =  A<int, 3> ;
  
};

template <typename T>
using U3 = A<T, 3>;
using Uint3 = A<int, 3>;
template <typename T>
struct E : public U3<T> {};
struct F: public A<int,4> {};
template <typename T1, typename T2>
struct C{
  T1 x;
  T2 y;
};
template <int x> struct B{
  int u[x];
};

int main()
{
  U3<int> u;
  A<int ,3> a;
  B<3> b;
  C<int, double> c;
  D d;
  E<unsigned int> e;
  F f;
  f.foo(2);
  printf("Size%lu %lu %lu %lu\n",
	 sizeof(u), sizeof(b) , sizeof(c) ,sizeof(d));
  
  return 0;
}
