#include <iostream>

extern "C" {
  void dl_tracer_trace() {
    std::cout << "dl_tracer_trace" << std::endl;
  }
}