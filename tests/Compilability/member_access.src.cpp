struct ClassA {
  int member;
};

void
func(ClassA a) {
  a.ClassA::member = 500;
  a.member = 100;
}
