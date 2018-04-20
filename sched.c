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
#include "coroutine.h"
#include <stdlib.h>
#include <sys/time.h>

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];

void switch_to(struct task_struct *next);

static unsigned int getmstime() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    perror("gettimeofday");
    exit(-1);
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static struct task_struct *pick() {
  int current_id  = current->id;
  int i;

  struct task_struct *next = NULL;

repeat:
  for (i = 0; i < NR_TASKS; ++i) {
    if (task[i] && task[i]->status == COROUTINE_SLEEP) {
      if (getmstime() > task[i]->wakeuptime)
        task[i]->status = COROUTINE_RUNNING;
    }
  }

  i = current_id;
  
  while(1) {
    i = (i + 1) % NR_TASKS;
    if (i == current_id) {
      // 循环了一圈说明没找到可被调度的线程
      goto repeat;
    }
    if (task[i] && task[i]->status == COROUTINE_RUNNING) {
      next = task[i];
      break;
    }
  } 
  
  return next;
}



void schedule() {
    struct task_struct *next = pick();
    if (next) {
      switch_to(next);
    }
}

void mysleep(int seconds) {
  current->wakeuptime = getmstime() + 1000*seconds;
  current->status = COROUTINE_SLEEP;
  schedule();
}
