class ClassA {
public:
  ClassA operator--() {
    --i;
    return *this;
  }
  const ClassA operator--(int) {
    ClassA temp(*this);
    --*this;
    return temp;
  }

private:
  int i;
};
