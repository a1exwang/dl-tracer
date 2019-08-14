#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

#include <libunwind.h>

extern "C" ssize_t read(int __fd, void *__buf, size_t __nbytes);
extern "C" void read_end();

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