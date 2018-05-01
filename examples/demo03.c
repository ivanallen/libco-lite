#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/epoll.h>
#include "coroutine.h"

void usage() {
    printf("usage:\n\t./demo02 ip port\n");
    exit(-1);
}

void *do_client(void *arg) {
    int clifd = (int)(long)arg;
    int n = 0;
    char buf[64] = { 0 };
    while(1) {
        printf("ready to co_read:%d\n", clifd);
        n = co_read(clifd, buf, 63);
        if (n < 0) {
            perror("co_read");
            exit(-1);
        }
        if (n == 0) {
            printf("connection closed\n");
            break;
        }
        buf[n] = '\0';
        printf("recv from %d:%s\n", clifd, buf);
        n = co_write(clifd, buf, n);
        if (n < 0) {
            perror("co_write");
            exit(-1);
        }
    }
}

void *my_accept(void *arg) {
    int listenfd = *(int *)arg;
    while(1) {
        int clifd = co_accept(listenfd, NULL, NULL);
        printf("%d connect\n", clifd);
        if (clifd < 0) {
            perror("co_accept");
            exit(-1);
        }
        int cid = 0;
        co_create(&cid, do_client, (void*)clifd);
        // cid 何时释放？
    }
}

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    short port = 8002;
    if (argc > 1) {
       ip = argv[1]; 
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    printf("listen %s:%d\n", ip, port);
    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int ret = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
        perror("bind");
        exit(-1);
    }
    int accept_cid;
    listen(listenfd, 5);
    co_create(&accept_cid, my_accept, &listenfd);
    co_eventloop();
    close(listenfd);
    co_join(accept_cid);
    return 0;
}
