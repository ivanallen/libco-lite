# 1 "switch.S"
# 1 "<built-in>"
# 1 "<命令行>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<命令行>" 2
# 1 "switch.S"


.section .text
.global switch_to
switch_to:
# 38 "switch.S"
  pushq %rbp
  movq %rsp, %rbp


  pushq %rdi
  pushq %rsi
  pushq %rbx
  pushq %rdx
  pushq %rcx
  pushq %rax
  pushfq
  pushq %r8
  pushq %r9
  pushq %r12
  pushq %r13
  pushq %r14
  pushq %r15


  movq current, %rax
  movq %rsp, 8(%rax)
  movq %rdi, %rax
  movq %rax, current
  movq 8(%rax), %rsp

  popq %r15
  popq %r14
  popq %r13
  popq %r12
  popq %r9
  popq %r8
  popfq
  popq %rax
  popq %rcx
  popq %rdx
  popq %rbx
  popq %rsi
  popq %rdi

  popq %rbp
  ret
