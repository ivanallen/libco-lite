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
#include <stdio.h>
#include <stdlib.h>
#include "cosyscall.h"
#include "coroutine.h"

struct thread_env_t *co_get_thread_env();

pid_t gettid() {
   pid_t tid;
   tid = syscall(SYS_gettid);
}

/**
 * 将当前任务添加到阻塞队列
 */
int add_queue(int fd, int events) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *current;
    int epfd = thread_env->epoll.epfd;

    event.events = events;
    event.data.fd = fd;

    current = thread_env->current;
    current->status = COROUTINE_WAIT; // 设置为阻塞状态
    current->revents[fd] &= ~events;

    if (current->events[fd].data.fd <= 0) {
        // 从未监听过此描述符，第一次加入 epoll 监控
        current->events[fd].data.fd = fd;
        current->events[fd].events = EPOLLIN;
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
        if (ret < 0) {
            return ret;
        }
    } else if (current->events[fd].events & events != current->events[fd].events) {
        // 未监听过 events 中的某些事件， 需要修改修改监控项
        current->events[fd].events |= events;
        event.events |= current->events[fd].events;
        int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
        if (ret < 0) {
            return ret;
        }
    }

    // 将 current 插入阻塞队列，一旦描述符上有事件响应，则激活这个任务
    TAILQ_INSERT_TAIL(&(thread_env->epoll.block_queue[fd]), current, block_entry);
    return 0;
}


int co_accept(int listenfd, struct sockaddr *addr, socklen_t *len) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *current;
    int epfd = thread_env->epoll.epfd;
    int ret = -1;

    event.events = EPOLLIN;
    event.data.fd = listenfd;

    current = thread_env->current;
    current->status = COROUTINE_WAIT; // 设置为阻塞状态

    if ((ret = add_queue(listenfd, EPOLLIN)) < 0) {
        return ret;
    }
    
    current->fds[current->fds_idx++] = listenfd; // 当前协程阻塞在此 fd 上

    while(!(current->revents[listenfd] & EPOLLIN)) {
        current->status = COROUTINE_WAIT;
        schedule();
    }
   
    LOG_DEBUG("accept\n");
    return accept(listenfd, addr, len);
}

int co_read(int fd, void *buf, size_t len) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *current;
    int epfd = thread_env->epoll.epfd;
    int ret = -1;

    event.events = EPOLLIN;
    event.data.fd = fd;

    current = thread_env->current;
    current->status = COROUTINE_WAIT; // 设置为阻塞状态

    if ((ret = add_queue(fd, EPOLLIN)) < 0) {
        return ret;
    }

    current->fds[current->fds_idx++] = fd; // 当前协程阻塞在此 fd 上

    while(!(current->revents[fd] & EPOLLIN)) {
        current->status = COROUTINE_WAIT;
        schedule();
    }

    LOG_DEBUG("read\n");
    return read(fd, buf, len);
}

int co_write(int fd, void *buf, size_t len) {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event event;
    struct task_struct_t *current;
    int epfd = thread_env->epoll.epfd;
    int ret = -1;

    event.events = EPOLLOUT;
    event.data.fd = fd;

    current = thread_env->current;
    current->status = COROUTINE_WAIT; // 设置为阻塞状态

    if ((ret = add_queue(fd, EPOLLOUT)) < 0) {
        return ret;
    }

    while(!(current->revents[fd] & EPOLLOUT)) {
        current->status = COROUTINE_WAIT;
        schedule();
    }

    // 抹掉 EPOLLOUT 事件，否则每次事件循环都会被唤醒
    current->events[fd].events &= ~EPOLLOUT;
    current->revents[fd] &= ~EPOLLOUT;

    ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &(current->events[fd]));
    if (ret < 0) {
        return ret;
    }

    LOG_DEBUG("write\n");
    return write(fd, buf, len);
}
