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

#include <sys/epoll.h>
#include "cosyscall.h"
#include "coroutine.h"

struct thread_env_t *co_get_thread_env();

pid_t gettid() {
   pid_t tid;
   tid = syscall(SYS_gettid);
}


int co_accept(int listenfd, struct sockaddr *addr, socklen_t *len) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *tsk;
    int epfd = thread_env->epoll.epfd;

    event.events = EPOLLIN;
    event.data.fd = listenfd;


    thread_env->current->status = COROUTINE_WAIT; // 设置为阻塞状态
    thread_env->current->fd = listenfd;

    if (thread_env->epoll.task[listenfd]) {
        schedule();
        return accept(listenfd, addr, len);
    };

    thread_env->epoll.task[listenfd] = thread_env->current;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    if (ret < 0) {
        return ret;
    }

    schedule();

    // puts("accept");
    return accept(listenfd, addr, len);
}

int co_read(int fd, void *buf, size_t len) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *tsk;
    int epfd = thread_env->epoll.epfd;

    event.events = EPOLLIN;
    event.data.fd = fd;

    thread_env->current->status = COROUTINE_WAIT; // 设置为阻塞状态
    thread_env->current->fd = fd;
    if (thread_env->epoll.task[fd]) {
        schedule();
        return read(fd, buf, len);
    };
    thread_env->epoll.task[fd] = thread_env->current;

    // puts("co_read:epoll_ctl add");
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0) {
        return ret;
    }

    schedule();

    // puts("read");
    return read(fd, buf, len);
}
