struct {
  int a;
  int b;
} obj1, obj2;

int
main() {
  obj2.a = 100;
  obj1 = obj2;
  return 0;
}
