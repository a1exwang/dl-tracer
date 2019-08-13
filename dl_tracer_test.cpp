#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

#include <libunwind.h>

extern "C" ssize_t read(int __fd, void *__buf, size_t __nbytes);
extern "C" void read_end();

void register_symbols() {
  unw_dyn_info_t info;
  info.start_ip = (unw_word_t)read;
  info.end_ip = (unw_word_t)read_end;
  info.format = UNW_INFO_FORMAT_DYNAMIC;
  info.gp = 0;
  auto *table_info = &info.u.ti;
  table_info->name_ptr = (unw_word_t)"read1";
//  table_info->segbase =

  size_t op_count = 32;
  unw_dyn_region_info_t *region = new unw_dyn_region_info_t + sizeof(unw_dyn_op_t) * (op_count - 1);
  region->next = nullptr;
  region->insn_count = 10;
  region->op_count = op_count;

//  pi->regions = region;

//  _U_dyn_register(&info);
}

int main() {
  char buf[1024];
  memset(buf, 0, sizeof(buf));

  int fd = open("a", O_RDONLY);
  if (fd < 0) {
    perror("open");
    abort();
  }

  auto nread = read(fd, buf, sizeof(buf) - 1);
  if (nread < 0) {
    perror("read");
    abort();
  }

  printf("read '%s'\n", buf);
  return 0;
}