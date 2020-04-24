#include <stdio.h>

template <typename T, int x>
T funcX(T y)
{
  T a[x];
  int z;
  int i;
  for(i = 0 ; i < x; i++){
    a[i] = i;
  }
  for(i = 0, z = 0; i < x; i++){
    z += a[i];
  }
  return z;
}
template <typename T1, typename T2>
T2 funcY(T1 x, T2 y)
{
  return  x+y;
}
  
int main()
{
  printf("%d\n", funcX<int, 3>(4));
  funcY<int, double> (1, 2.0);
  
  return 0;
}
