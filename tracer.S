.text
    .global enter_trace
    .global leave_trace
    .global stack_dump
    .global read1
    .global read_original

read1:
  push %rbp
  mov %rsp, %rbp

  push %rax
  push %rcx
  push %rdx
  push %rsi
  push %rdi
  push %r8
  push %r9
  push %r10
  push %r11

  push %rbx
  push %r12
  push %r13
  push %r14
  push %r15

  /*if (enter_trace() == 0) {
    stack_dump();
    leave_trace();
  }
  return read_original(fd, data, size);*/

  call enter_trace@plt
  cmp $0, %eax
  jne .read.exit

  call stack_dump@plt
  call leave_trace@plt

.read.exit:
  pop %r15
  pop %r14
  pop %r13
  pop %r12
  pop %rbx

  pop %r11
  pop %r10
  pop %r9
  pop %r8
  pop %rdi
  pop %rsi
  pop %rdx
  pop %rcx
  pop %rax

  mov %rbp, %rsp
  pop %rbp

  lea read_original@GOTPCREL(%rip), %rax
  mov (%rax), %rax

  mov (%rax), %rax
  jmp *%rax


