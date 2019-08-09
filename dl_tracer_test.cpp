#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

#include <libunwind.h>

extern "C" ssize_t read1(int __fd, void *__buf, size_t __nbytes);
extern "C" void read1_end();

int main() {
  int a;
  char buf[1024];
  memset(buf, 0, sizeof(buf));

  int fd = open("a", O_RDONLY);
  if (fd < 0) {
    perror("open");
    abort();
  }

  unw_dyn_info_t info;
  info.start_ip = (unw_word_t)read1;
  info.end_ip = (unw_word_t)read1_end;
  info.format = UNW_INFO_FORMAT_DYNAMIC;
  info.gp = 0;
  auto *pi = &info.u.pi;
  pi->name_ptr = (unw_word_t)"read1";
  pi->handler = 0;
  pi->flags = 0;

  size_t op_count = 32;
  unw_dyn_region_info_t *region = new unw_dyn_region_info_t + sizeof(unw_dyn_op_t) * (op_count - 1);
  region->next = nullptr;
  region->insn_count = 10;
  region->op_count = op_count;
//  region->op[0]

  pi->regions = region;

  _U_dyn_register(&info);

  auto nread = read1(fd, buf, sizeof(buf));
  if (nread < 0) {
    perror("read");
    abort();
  }

  printf("read '%s'\n", buf);
  return 0;
}