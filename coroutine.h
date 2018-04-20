#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#define NR_TASKS 1024 
#define STACK_SIZE 1024*128 // 32bit 128*4K  64bit 128*8K 

#define COROUTINE_RUNNING 0
#define COROUTINE_SLEEP 1
#define COROUTINE_EXIT 2

struct task_struct {
  int id;
  unsigned int wakeuptime; // 线程唤醒时间
  void *esp; // 保存 esp
  void (*th_fn)();
  int status;
  void* stack[STACK_SIZE];
};


int co_create(int *tid, void (*start_routine)());
int co_join(int tid);
void mysleep(int seconds);

#endif //__COROUTINE_H__
