void
func(int func_i, int func_j) {
  int func_x;
}

namespace ns {

void
ns_func(int ns_func_i) {
  int ns_func_x;
}

} // namespace ns

extern "C" {

void
extC_func(int extC_func_i) {
  int extC_func_x;
}
}

class Record {
  void
  Record_memfn(int Record_memfn_i) {
    int Record_memfn_x;
  }
  int Record_field;
};
