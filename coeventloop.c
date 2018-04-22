#include <sys/epoll.h>
#include <stdlib.h>
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
        // printf("event come in nready:%d\n", nready);
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
            // printf("event come in:%d\n", ev->data.fd);
            if (ev->events | EPOLLIN) {
                tsk = thread_env->epoll.task[ev->data.fd];
                tsk->status = COROUTINE_RUNNING; // 可被调度 
            }
        }
        schedule();
    }
}
