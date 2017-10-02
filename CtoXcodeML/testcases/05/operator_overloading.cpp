class Ratio {
public:
  Ratio(int x, int y) {
    a = x;
    b = y;
  }
  int
  numer() const {
    return a;
  }
  int
  denom() const {
    return b;
  }
  Ratio operator++(int);
  Ratio operator++() {
    a += b;
    return *this;
  }
  Ratio operator--(int);
  Ratio operator--() {
    a -= b;
    return *this;
  }

private:
  int a, b;
};

Ratio Ratio::operator++(int) {
  Ratio self(*this);
  a += b;
  return self;
}

Ratio Ratio::operator--(int) {
  Ratio self(*this);
  a -= b;
  return self;
}

Ratio operator-(const Ratio &a) {
  return Ratio(-a.numer(), a.denom());
}

Ratio operator+(const Ratio &a, const Ratio &b) {
  return Ratio(
      a.numer() * b.denom() + a.denom() * b.numer(), a.denom() * b.denom());
}

Ratio operator-(const Ratio &a, const Ratio &b) {
  return a + (-b);
}

Ratio operator*(const Ratio &a, const Ratio &b) {
  return Ratio(a.numer() * b.numer(), a.denom() * b.denom());
}

Ratio operator/(const Ratio &a, const Ratio &b) {
  return a * Ratio(b.denom(), b.numer());
}

Ratio operator%(const Ratio &a, const Ratio &b) {
  return Ratio(0, 1);
}
