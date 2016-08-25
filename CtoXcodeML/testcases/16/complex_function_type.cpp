double (*f(int (*p)(int))) (double) {
  return (double(*)(double))p;
}
