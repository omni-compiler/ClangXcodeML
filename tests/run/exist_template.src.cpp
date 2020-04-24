#include <stdio.h>

struct B{
  int f(){
    printf("HOGE\n");
    return 0;
  };
};
struct C{
  int f()
  {
    printf("FUGA\n");
    return 1;
  }
};

template <bool v, typename T1, typename T2>
struct if_;

template <typename T1, typename T2>
struct if_<false, T1, T2>{
  using type = T2;
};

template <typename T1, typename T2>
struct if_<true, T1, T2>{
  using type = T1;
};
struct true_type{static constexpr bool value = true;};
struct false_type{static constexpr bool value = false;};  

template <typename T>
struct exist_f {
private:
  template<typename S, int (S::*)() = &S::f>
  static true_type check(S*);
  static false_type check(...);
public:
  static constexpr bool value = decltype(check((T*)nullptr))::value;
};

int main()
{
  if_<exist_f<B>::value, B, C>::type b;
  if_<exist_f<int>::value, B, C>::type c;
  b.f();
  c.f();
  return 0;
}
