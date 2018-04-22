#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/epoll.h>

void usage() {
    printf("usage:\n\t./demo02 ip port\n");
    exit(-1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usage();
    }
    char *ip = argv[1];
    short port = atoi(argv[2]);
    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        perror("bind");
        exit(-1);
    }
    listen(listenfd, 5);

    int epfd = epoll_create(1);
    if (epfd < 0) {
        perror("epoll_create");
        exit(-1);
    }

    struct epoll_event event = { 0 };
    struct epoll_event events[16] = { 0 };
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);

    int i;

    while(1) {
        int nready = epoll_wait(epfd, events, 16, -1);
        if (nready < 0) {
            perror("nready");
            exit(-1);
        }
        for (i = 0; i < nready; ++i) {
            struct epoll_event *ev = &events[i];
            if (ev->data.fd == listenfd) { // 有新连接
                int clifd = accept(listenfd, NULL, NULL);
                printf("%d come in\n", clifd);
                event.data.fd = clifd;
                event.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clifd, &event);
            } else {
                char buf[64] = { 0 };
                int n = read(ev->data.fd, buf, 64);
                if (n == 0) {
                    printf("%d: %s\n", ev->data.fd, "closed");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ev->data.fd, NULL);
                } else {
                    printf("%d: %s\n", ev->data.fd, buf);
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
