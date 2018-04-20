#include <stdio.h>
#include <stdlib.h>
#include "coroutine.h"


void fun1() {
  int i = 5;
  while(i--) {
    printf("hello, I'm fun1\n");
    mysleep(4);
  }
}

void fun2() {
  int i = 10;
  while(i--) {
    printf("hello, I'm fun2\n");
    mysleep(1);
  }
}

void fun3() {
  int i = 2;
  while(i--) {
    printf("hello, I'm fun3\n");
    mysleep(1);
  }
}


int main() {
  
  int tid1, tid2, tid3;
  co_create(&tid1, fun1);
  printf("create co %d\n", tid1);
  co_create(&tid2, fun2);
  printf("create co %d\n", tid2);
  co_create(&tid3, fun3);
  printf("create co %d\n", tid3);
  
  int i = 2;
  while(i--) {
    printf("hello, I'm main\n");
    mysleep(3);
  }
  co_join(tid1);
  co_join(tid2);
  co_join(tid3);

  printf("over!\n");
  return 0;
}


