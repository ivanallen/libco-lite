/*void switch_to(struct task_struct *next)*/

.section .text
.global switch_to
switch_to:
#if defined(__i386__)
  pushl %ebp
  movl %esp, %ebp /* 更改栈帧，以便寻参 */

  /* 保存现场 */
  pushl %edi
  pushl %esi
  pushl %ebx
  pushl %edx
  pushl %ecx
  pushl %eax
  pushfl
 
  /* 准备切换栈 */
  movl current, %eax /* 取 current 基址放到 eax */
  movl %esp, 8(%eax) /* 保存当前 esp 到线程结构体 */ 
  movl 8(%ebp), %eax /* 取下一个线程结构体基址*/
  movl %eax, current /* 更新 current */
  movl 8(%eax), %esp /* 切换到下一个线程的栈 */

  /* 恢复现场, 到这里，已经进入另一个线程环境了，本质是 esp 改变 */
  popfl
  popl %eax
  popl %edx
  popl %ecx
  popl %ebx
  popl %esi
  popl %edi

  popl %ebp
  ret
#elif defined(__x86_64__)
  pushq %rbp
  movq %rsp, %rbp
 
  /* 保存现场 */
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

  /* 准备切换栈 */
  movq current, %rax
  movq %rsp, 8(%rax)
  movq %rdi, %rax /* switch_to 的参数 */
  movq %rax, current
  movq 8(%rax), %rsp /* 切换到下一个协程 */

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
#endif
