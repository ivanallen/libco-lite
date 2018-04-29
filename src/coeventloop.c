#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include "coroutine.h"


struct thread_env_t *co_get_thread_env();

int co_eventloop() {
    struct thread_env_t *thread_env = co_get_thread_env();
    struct epoll_event events[1024] = { 0 };
    int epfd = thread_env->epoll.epfd;
    int nready = -1;
    int i = 0;
    while(1) {
        nready = epoll_wait(epfd, events, 1024, 1000);
        // LOG_DEBUG("event come in nready:%d\n", nready);
        if (nready < 0) {
            perror("epoll_wait");
            exit(1);
        }
        if (nready == 0) {
            schedule();
            continue;
        }
        
        // printf("process event\n");
        struct epoll_event *ev;
        struct task_struct_t *tsk;
        for(i = 0; i < nready; ++i) {
            ev = &events[i];
            LOG_DEBUG("event come in:%d events:%d\n", ev->data.fd, ev->events);
            // tsk = thread_env->epoll.task[ev->data.fd];
            tsk = TAILQ_FIRST(&thread_env->epoll.block_queue[ev->data.fd]);
            TAILQ_REMOVE(&thread_env->epoll.block_queue[ev->data.fd], tsk, block_entry);
            if (tsk->events[ev->data.fd].events & ev->events) {
                tsk->status = COROUTINE_RUNNING; // 可被调度 
                tsk->revents[ev->data.fd] = ev->events;
            }
        }
        schedule();
    }
}
