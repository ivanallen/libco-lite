/*
 * Copyright (c) 2018, kingchin1218@126.com 
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
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdlib.h>

#define PORT 9999
#define MAXSIZE 1000
#define LOW_UP_SETOFF ('A' - 'a')

int handle(int fd);
size_t upper(char buf[], int n);

/**
*returned -1 means peer socket closed,otherwise means the len of buf
*/
int get_line(int fd, char *buf, size_t len) {
        size_t n = 0;
        int curr = 0;
        do {
                n = read(fd, buf + curr, len);
                curr += n;
        } while(n != 0 && buf[curr - 1]  != '\n');

        if (n == 0) {
                return -1;
        }

        return curr;
}

int handle(int fd) {
        char buf[MAXSIZE];

        size_t n = get_line(fd, buf, MAXSIZE);
        buf[n] = '\n';

        if (0 == n) {
                return -1;
        }

        n = upper(buf, n);
        write(fd, buf, n);

        return n;
}

size_t upper(char buf[], int n) {
	int i = 0;

	while (i < n) {
		if ('\n' == buf[i]) {
			break;		
		}
		
		buf[i] = buf[i] + LOW_UP_SETOFF;
		++i;
	} 

	return i+1;
}

int main (int argc, char **argv) {
	
	int ret;

	//server socket init
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	
	bzero(&servaddr, sizeof(servaddr));
	

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	
	ret = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if (ret != 0) {
		perror("bind error\n");
		exit(-1);
	}

	ret = listen(listenfd, 5);
	if (ret != 0) {
		perror("listen error\n");
		exit(-1);
	}

	printf("serv:listening......\n");	


	//epoll init
	struct epoll_event event;
	struct epoll_event wait_events[10];

	int epollfd = epoll_create(10);	
	if (epollfd < 0) {
		perror("epoll create error.\n");
		exit(-1);
	}
	
	event.data.fd = listenfd;
	event.events = EPOLLIN;
	
	ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
	if (ret != 0) {
		perror("epoll ctl error\n");
		exit(-1);
	}

	printf("serv:epoll created.\n");
	
	//loop	
	while (1) {
		int nready = epoll_wait(epollfd, wait_events, 10, -1);
		
		printf("serv:epoll_wait returned.\n");

		int i = 0;
		for (i = 0; i < nready; ++i) {
			//server
			if ((listenfd == wait_events[i].data.fd) && (EPOLLIN == (EPOLLIN & wait_events[i].events)) ) {
							
				printf("serv: a client accepted.\n");
	
				struct sockaddr_in cliaddr;
				int clilen = sizeof(cliaddr);
			
				int clientfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
				event.data.fd = clientfd;
				event.events = EPOLLIN;
				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);		
			}
			//client
			else if (wait_events[i].events & EPOLLIN) {
				
				printf("serv: a msg from client.\n");
				
				ret = handle(wait_events[i].data.fd);
				if (ret <= 0) {
					epoll_ctl(epollfd, EPOLL_CTL_DEL, wait_events[i].data.fd, NULL);
					close(wait_events[i].data.fd);
				}
			}
			
		}
		
	}


	return 0;
}
