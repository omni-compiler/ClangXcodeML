double (*f(int (*p)(int))) (double) {
  return (double(*)(double))f;
}
