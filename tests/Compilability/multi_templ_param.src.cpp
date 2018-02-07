template <typename FuncT, typename ReturnT, typename ParamT1, typename ParamT2>
ReturnT
apply(FuncT func, ParamT1 x, ParamT2 y) {
  return func(x, y);
}
