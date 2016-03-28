namespace A {
  int x;
  enum B {
    E, N, U, M
  };
}

namespace B {
  class C {
    public:
    int x;
  };
  const int override = 108;
  int f() {
    int answer = 42;
    return answer;
  }
};

class D {
  class E {
    public:
      int y;
  };
};

namespace F {
  namespace G {
    class H;
  }
}

class F::G::H { };
