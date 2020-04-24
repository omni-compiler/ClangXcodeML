#include <stdio.h>
template <typename... Arg>class A{
public:
  A<>(){printf("END\n");};
};



template <typename... Arg>
class A<int , Arg...>: public A <Arg...>
{
public:
  A(int , Arg... arg):A <Arg...>(arg...){ printf("INT\n");};
  
};

template <typename... Arg>
class A<double, Arg...>: public A <Arg...>
{
public:
  A(double , Arg... arg):A <Arg...>(arg...){ printf("Double\n");};
};
   

int main()
{
  A<int, double, int, int, double> a(1,2,3,4,5);
  return 0;
}

  
