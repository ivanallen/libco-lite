/*
 * Copyright (c) 2018, Allen. 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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


