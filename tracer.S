.text
    .global enter_trace
    .global leave_trace
    .global stack_dump
    .global read
    .global read_end
    .global read_original
    .global my_get_rbp

read:
  push %rbp
  mov %rsp, %rbp

  push %rdx
  push %rsi
  push %rdi
  push %rcx
  push %r8
  push %r9

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
  pop %r9
  pop %r8
  pop %rcx
  pop %rdi
  pop %rsi
  pop %rdx

  mov %rbp, %rsp
  pop %rbp

  lea read_original@GOTPCREL(%rip), %rax
  mov (%rax), %rax

  mov (%rax), %rax
  jmp *%rax
read_end:
  nop
  ret

my_get_rbp:
  mov %rbp, %rax
  ret
