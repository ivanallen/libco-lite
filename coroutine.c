/*
 * Copyright (c) 2018, ivan_allen@163.com 
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
#include "cosyscall.h"
#include <stdlib.h>

void schedule();

// static struct task_struct_t init_task = {0, NULL, COROUTINE_RUNNING, 0, 0, {0}};
static struct task_struct_t init_task = {0, 0, NULL, NULL, NULL, NULL, COROUTINE_RUNNING, {0}};

struct task_struct_t *current = &init_task;

struct thread_env_t *g_thread_env_arr[10240] = {0}; // 最多 10240 个线程
// struct task_struct_t *task[NR_TASKS] = {&init_task,};

// 线程启动函数
static void start(struct task_struct_t *tsk) {
    tsk->co_fn();
    tsk->status = COROUTINE_EXIT;
    schedule();
}

/**
 * 创建线程环境块
 * 说明：如果存在就直接返回
 */
struct thread_env_t *co_get_thread_env() {
    pid_t tid = gettid();
    struct thread_env_t *thread_env = g_thread_env_arr[tid];
    struct task_struct_t *init_task = NULL;
    if (!thread_env) {
        thread_env = (struct thread_env_t*)calloc(sizeof(struct thread_env_t), 1);
        thread_env->task_count = 1;
        // 创建主协程，即初始任务。
        init_task = (struct task_struct_t*)calloc(sizeof(struct task_struct_t), 1);
        // init_task 其它字段全部为空。
        init_task->thread_env = thread_env;
        init_task->status = COROUTINE_RUNNING;

        thread_env->task[0] = init_task;
        thread_env->current = init_task;
        g_thread_env_arr[tid] = thread_env;
    }
    return thread_env;
}

int co_create(int *cid, void (*start_routine)()) {
    int id = -1;
    // 获取当前线程环境块
    struct thread_env_t *thread_env = co_get_thread_env();

    // 查找任务队列中空闲位置
    while(++id < NR_TASKS && (thread_env->task[id] != NULL));
    if (id == NR_TASKS) return -1;
    if (cid) *cid = id;

    // 创建协程控制块
    struct task_struct_t *tsk = (struct task_struct_t*)malloc(sizeof(struct task_struct_t));
    thread_env->task[id] = tsk;

    tsk->id = id;
    tsk->co_fn = start_routine;
    tsk->arg = NULL;
    void **stack = tsk->stack; // 栈顶界限
    tsk->wakeuptime = 0;
    tsk->status = COROUTINE_RUNNING;

    // 初始化 switch_to 函数栈帧
    // 下面这些填充实际是无意义的，可以配合调试用
#if defined(__x86_64__)
    tsk->esp = (void *)(stack+STACK_SIZE-16);
    stack[STACK_SIZE-16] = (void *)15; // r15
    stack[STACK_SIZE-15] = (void *)14; // r14
    stack[STACK_SIZE-14] = (void *)13; // r13
    stack[STACK_SIZE-13] = (void *)12; // r12
    stack[STACK_SIZE-12] = (void *)9; // r9
    stack[STACK_SIZE-11] = (void *)8; // r8
    stack[STACK_SIZE-10] = (void *)7; // eflags
    stack[STACK_SIZE-9] = (void *)6; // rax
    stack[STACK_SIZE-8] = (void *)5; // rcx
    stack[STACK_SIZE-7] = (void *)4; // rdx
    stack[STACK_SIZE-6] = (void *)3; // rbx
    stack[STACK_SIZE-5] = (void *)2; // rsi
    stack[STACK_SIZE-4] = (void *)tsk; // rdi // start 函数的参数
    stack[STACK_SIZE-3] = (void *)0; // old ebp
    stack[STACK_SIZE-2] = (void *)start; // ret to start
    // start 函数栈帧，刚进入 start 函数的样子 
    stack[STACK_SIZE-1] = (void *)100;// ret to unknown，如果 start 执行结束，表明线程结束
#elif defined(__i386__)
    tsk->esp = (void *)(stack+STACK_SIZE-11);
    stack[STACK_SIZE-11] = (void *)7; // eflags
    stack[STACK_SIZE-10] = (void *)6; // eax
    stack[STACK_SIZE-9] = (void *)5; // ecx
    stack[STACK_SIZE-8] = (void *)4; // edx
    stack[STACK_SIZE-7] = (void *)3; // ebx
    stack[STACK_SIZE-6] = (void *)2; // esi
    stack[STACK_SIZE-5] = (void *)1; // edi
    stack[STACK_SIZE-4] = (void *)0; // old ebp
    stack[STACK_SIZE-3] = (void *)start; // ret to start
    // start 函数栈帧，刚进入 start 函数的样子 
    stack[STACK_SIZE-2] = (void *)100;// ret to unknown，如果 start 执行结束，表明线程结束
    stack[STACK_SIZE-1] = (void *)tsk; // start 的参数
#endif

    return 0;
}

int co_join(int cid) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct task_struct_t **task = thread_env->task;
    while(task[cid]->status != COROUTINE_EXIT) {
        schedule();
    }
    printf("thread_env:%p cid:%d\n", thread_env, cid);
    free(task[cid]); // TODO: 这一行会导致 core，待查
    task[cid] = NULL;
}
