#include <backward.hpp>

extern "C" {
  void stack_dump() {
    printf("stack dump\n");
    using namespace backward;
    StackTrace st; st.load_here(32);
    Printer p; p.print(st);
  }
}