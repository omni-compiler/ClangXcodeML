union Union1 {
  Union1(double, double) {
  }

  int i;
  short s;
};

const Union1 union1a(3.14, 2.78);

int
main() {
  const Union1 union1b = union1a;
  const Union1 union1c(3.14, 2.78);
}
