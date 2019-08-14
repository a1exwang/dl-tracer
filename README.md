# dl-tracer

`dl-tracer` is a __library call tracer generator__ on Linux.

### Use Case/Why dl-tracer?

For example, when you are debugging an extremely large project, which consist
of tens of thousands of source files and the debug build result might be 
several GBs(Yes, I'm looking at you, TensorFlow).

What I'd like to debug:
- Show where is a certain function called.
- Maybe inject a stack trace at anywhere I like.
- Log all call site of a function into a file for further analysis.

The common runtime debugging tools does not suit well on local machine because:

- `printf` debug
    - Recompiling is too slow. The final linking process might take 5 minutes or so.
- gdb or other interactive debuggers
    - If we do not load the symbols, it is hard to use gdb.
    - If we do load the debug symbols, the symbol loading itself takes several minutes.
    - Even if the symbol loading is not a problem, gdb will slow down the process dramatically.
- Debug with logger outputs
    - Not flexible enough. Sometime you want to know the value of a variable but no log is associated with the variable.
- ltrace/strace
    - Not flexible enough.
    - If you write ltrace hooks I should suit well in this case. But `ltrace` code base is too old and it's been years since its latest update. And I used to try but I gave that up finally.
    - Also note that `ptrace` still affects the performance if that's your concern.

So finally I found the `LD_PRELOAD` way is best suit for my case.
That is __writing a separate shared library that implements the function you want to trace, and inject the library to the target application with LD_PRELOAD__.

##### Pros
- Not intrusive. No need the recompile the target application, only the trace library which should be quite fast.
- Fast and Precise. Won't slow down unrelated code.
- Flexible. You can do anything with arbitrary C/C++ code when a function is called.

##### Cons
- Cannot use interactively like gdb
- Can only trace library function calls(through PLT). May not trace C `static` functions.


### How to use

Say, you wanna trace all the function call to the glibc 
`read` in the application `build/dl_tracer_test`.
And print a stack trace at the call site.

##### 1. Generate the tracer library
```bash
$ python3 generate_tracer.py --mode so --output libtrace.so --symbols read
```

##### 2. Load the library to the target application

```bash
$ cat dl_tracer_test.cpp
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>

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
$ mkdir build && cd build && cmake .. && make
$ env LD_PRELOAD=./libtrace.so build/dl_tracer_test
```

Output

```
stack_dump_with_libunwind
0x7f6780671a27:  (stack_dump+0x9)
0x7f678067fee2:  (_ZL14my_tracer_codev+0x2e)
0x55ef0668a23e:  (main+0x85)
0x7f67800f7ee3:  (__libc_start_main+0xf3)
0x55ef0668a0ee:  (_start+0x2e)
my trace code
read ''
```

So we manage to inject code to the `read` libc call.

If we change the tracer code. We only need to recompile the tracer, not `dl_tracer_test.cpp`.

### Use it as a code generator

Also, you can use dl_tracer as a code generator.

##### Generate the tracer source code

```bash
$ python3 generate_tracer.py --mode src --output src1 --symbols read
$ ls src1
read.tracer.cpp  read.tracer.S
$ cat read.tracer.cpp

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

void *read_original = 0;

__attribute__((constructor(200))) static
void my_initialize() {
  if (read_original == 0) {
    read_original = (void*)dlsym(RTLD_NEXT, "read");
  }
}

static void my_tracer_code();

extern "C" {

__attribute__((visibility("default")))
void read_tracer() {
    my_tracer_code();
}

}

#include <stdio.h>
static void my_tracer_code() {
    // Write your custom trace code here.
    printf("my trace code\n");
}

```

It will generate
```c++
// src1/read.tracer.cpp
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

void *read_original = 0;

__attribute__((constructor(200))) static
void my_initialize() {
  if (read_original == 0) {
    read_original = (void*)dlsym(RTLD_NEXT, "read");
  }
}

static void my_tracer_code();

extern "C" {

__attribute__((visibility("default")))
void read_tracer() {
    my_tracer_code();
}

}

#include <stdio.h>
static void my_tracer_code() {
    // Write your custom trace code here.
    printf("my trace code\n");
}
```

You can modify the tracer code as you like and compile with the generated `compile.sh`.
And inject into the target application just as the above example.
