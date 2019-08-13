#define BACKWARD_HAS_UNWIND 0
#define BACKWARD_HAS_BACKTRACE 1
#define BACKWARD_HAS_BFD 1
#include <backward.hpp>
#include <backtrace.h>
#include <cstdlib>
#include <execinfo.h>
#include <libunwind.h>
#include <bfd.h>

extern "C" {
void stack_dump_with_backward_cpp() {
  printf("stack_dump_with_backward_cpp\n");
  using namespace backward;
  StackTrace st;
  st.skip_n_firsts(3);
  st.load_here(32);
  Printer p;
  p.snippet = false;
  p.print(st);
}

void err_callback(void *data, const char *msg, int errnum) {
  fprintf(stderr, "backtrace err: errnum %d, err '%s'\n", errnum, msg);
  abort();
}
int stack_trace_callback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function) {
  printf("  %16lx %s:%d function %s\n", pc, filename, lineno, function);
  return 0;
}
void stack_dump_with_libbacktrace() {
  printf("stack_dump_with_libbacktrace\n");
  auto state = backtrace_create_state(nullptr, 1, err_callback, 0);
  backtrace_full(state, 0, stack_trace_callback, err_callback, 0);
}

#define BT_BUF_SIZE 100
void stack_dump_with_backtrace() {
  printf("stack_dump_with_backtrace\n");
  int j, nptrs;
  void *buffer[BT_BUF_SIZE];
  char **strings;

  nptrs = backtrace(buffer, BT_BUF_SIZE);
  printf("backtrace() returned %d addresses\n", nptrs);

  /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
     would produce similar output to the following: */

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (j = 0; j < nptrs; j++)
    printf("%s\n", strings[j]);

  free(strings);
}

int unw_slow_backtrace (void **buffer, int size, unw_context_t *uc);
void stack_dump_with_libunwind() {
  printf("stack_dump_with_libunwind\n");

  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);
//  void *buffer[1024];
//  int n = unw_slow_backtrace(buffer, sizeof(buffer) / sizeof(void*), &context);
  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    printf("0x%lx: ", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      printf(" (%s+0x%lx)\n", sym, offset);
    } else {
      printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}
void *my_get_rbp();
bool check_address_readable(void *pointer, size_t size) {
  int nullfd = open("/dev/random", O_WRONLY);

  bool ok = write(nullfd, pointer, size) >= 0;
  close(nullfd);
  return ok;
}

void stack_dump_frame_pointer() {
  std::cout << "stack_dump_frame_pointer" << std::endl;
  auto rbp = my_get_rbp();
  std::vector<void*> ret_addresses;
  // rbp -> old_rbp
  // rbp + sizeof(void*) -> ret_addr
  while (check_address_readable(rbp, 2 * sizeof(void*))) {
    auto older_rbp = *(void**)rbp;
    ret_addresses.push_back(*((void**)rbp + 1));
    rbp = older_rbp;
  }
  char **symbols = backtrace_symbols(ret_addresses.data(), ret_addresses.size());
  for (int i = 0; i < ret_addresses.size(); i++) {
    std::cout << ret_addresses[i] << " " << symbols[i] <<  std::endl;
  }
}

void stack_dump() {
  stack_dump_with_libbacktrace();
  stack_dump_with_backward_cpp();
  stack_dump_with_backtrace();
  stack_dump_with_libunwind();
  stack_dump_frame_pointer();
}

}