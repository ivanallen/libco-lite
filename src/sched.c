/*
 * Copyright (c) 2018, ivan_allen@163.com. 
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
#include <stdio.h>
#include <sys/time.h>

// extern struct task_struct_t *current;
// extern struct task_struct_t *task[NR_TASKS];
extern struct thread_env_t *g_thread_env_arr[10240];
struct thread_env_t *co_get_thread_env();

void switch_to(struct task_struct_t *current, struct task_struct_t *next);

struct task_struct_t *co_get_current() {
    struct thread_env_t *thread_env = co_get_thread_env();
    return thread_env->current;
}

static unsigned int getmstime() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        perror("gettimeofday");
        exit(-1);
    }
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static struct task_struct_t *pick() {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct task_struct_t *current = thread_env->current;
    struct task_struct_t **task = thread_env->task;

    int current_id  = current->id;
    int i;

    struct task_struct_t *next = NULL;

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
            // 循环了一圈说明没找到可被调度的线程, 这里很耗 cpu，需要修改
            if (current->status == COROUTINE_RUNNING) {
                return current;
            }
            goto repeat;
        }
        if (task[i] && task[i]->status == COROUTINE_RUNNING) {
            next = task[i];
            break;
        }
    } 

    return next;
}

void co_switch_to(struct task_struct_t *next) {
    // struct thread_env_t *thread_env = co_get_thread_env();
    struct thread_env_t *thread_env = next->thread_env;
    struct task_struct_t *current = thread_env->current;
    // 这一行 printf 可以清楚的看到切换时机
    // LOG_DEBUG("tsk %d(%d) ----> %d(%d)\n", current->id, current->status, next->id, next->status);
    if (current == next) {
        return;
    }
    thread_env->current = next; // 这一行千万别放到 switch_to 后面
    switch_to(current, next);
}

void schedule() {
    struct task_struct_t *next = pick();
    if (next) {
        co_switch_to(next);
    }
}

void co_sleep(int seconds) {
    pid_t tid = gettid();
    struct thread_env_t *thread_env = co_get_thread_env();
    thread_env->current->wakeuptime = getmstime() + 1000*seconds;
    thread_env->current->status = COROUTINE_SLEEP;
    schedule();
}
