int
func() {
  union {
    int a;
    int b;
  } obj;
  obj.a = 0;
  obj.b = 100;
  return obj.b;
}
