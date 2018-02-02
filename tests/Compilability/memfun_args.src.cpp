class Klass {
public:
  int
  plus(int a, int b, int c) {
    return a + b + c;
  }
};

void
func() {
  Klass obj;
  int i = obj.plus(100, 200, 300);
}
