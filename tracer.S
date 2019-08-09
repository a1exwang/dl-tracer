.text
    .global enter_trace
    .global leave_trace
    .global stack_dump
    .global read1
    .global read1_end
    .global read_original

read1:
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
read1_end:
  nop
  ret
