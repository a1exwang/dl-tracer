#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

extern "C" ssize_t read1(int __fd, void *__buf, size_t __nbytes);

int main() {
  int a;
  char buf[1024];
  memset(buf, 0, sizeof(buf));

  int fd = open("a", O_RDONLY);
  if (fd < 0) {
    perror("open");
    abort();
  }

  auto nread = read1(fd, buf, sizeof(buf));
  if (nread < 0) {
    perror("read");
    abort();
  }

  printf("read '%s'\n", buf);
  return 0;
}