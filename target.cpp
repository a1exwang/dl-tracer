#include <dlfcn.h>
#include <map>
#include <thread>
#include <mutex>
#include <libunwind.h>


extern "C" ssize_t read1(int __fd, void *__buf, size_t __nbytes);
extern "C" void read1_end();
static std::map<std::thread::id, int> prevent_reentry;
static std::mutex pr_lock;

extern "C" {

void stack_dump();

typedef int (*open_original_t)(const char *pathname, int flags, unsigned int mode);
static int (*open_original)(const char *pathname, int flags, unsigned int mode) = 0;

int open2(const char *pathname, int flags, unsigned int mode) {
  bool is_reentry{};
  {
    std::unique_lock<std::mutex> _(pr_lock);
    is_reentry = prevent_reentry.find(std::this_thread::get_id()) != prevent_reentry.end();
    if (!is_reentry) {
      prevent_reentry[std::this_thread::get_id()] = 1;
    }
  }

  if (is_reentry) {
    return open_original(pathname, flags, mode);
  }

  stack_dump();

  auto ret = open_original(pathname, flags, mode);

  {
    std::unique_lock<std::mutex> _(pr_lock);
    prevent_reentry.erase(std::this_thread::get_id());
  }

  return ret;
}

typedef int (*read_original_t)(int fd, void *data, size_t size);
int (*read_original)(int fd, void *data, size_t size) = 0;

int enter_trace() {
  std::unique_lock<std::mutex> _(pr_lock);
  bool is_reentry = prevent_reentry.find(std::this_thread::get_id()) != prevent_reentry.end();
  if (!is_reentry) {
    prevent_reentry[std::this_thread::get_id()] = 1;
  }
  return is_reentry;
}

void leave_trace() {
  std::unique_lock<std::mutex> _(pr_lock);
  prevent_reentry.erase(std::this_thread::get_id());
}

extern "C" ssize_t read(int __fd, void *__buf, size_t __nbytes);
extern "C" void read_end();

__attribute__((constructor(200))) static
void my_initialize() {
  if (open_original == nullptr) {
    open_original = (open_original_t)(dlsym(RTLD_NEXT, "open"));
  }
  if (read_original == nullptr) {
    read_original = (read_original_t)(dlsym(RTLD_NEXT, "read"));
  }

  printf("my initialize done\n");
}


}
