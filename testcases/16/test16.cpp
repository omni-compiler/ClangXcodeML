using int2int = int(*)(int);
using double2double = double(*)(double);

double (*f(int (*p)(int))) (double) {
  return (double(*)(double))f;
}
