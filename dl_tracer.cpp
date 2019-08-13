#include <iostream>
#include <dlfcn.h>
#include <map>
#include <thread>
#include <mutex>

static std::map<std::thread::id, int> prevent_reentry;
static std::mutex pr_lock;
extern "C" {

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
}