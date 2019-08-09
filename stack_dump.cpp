#define BACKWARD_HAS_UNWIND 0
#define BACKWARD_HAS_UNWIND 0
#define BACKWARD_HAS_BACKTRACE 1
#define BACKWARD_HAS_BFD 1
#include <backward.hpp>

extern "C" {
  void stack_dump() {
    printf("stack dump\n");
    using namespace backward;
    StackTrace st;
    st.skip_n_firsts(3);
    st.load_here(32);
    Printer p;
    p.snippet = false;
    p.print(st);
  }
}