#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>


#define PORT 9999
#define MAXSIZE 1000
#define LOW_UP_SETOFF ('A' - 'a')

int handle(int fd);
size_t upper(char buf[], int n);

int handle(int fd) {
	char buf[MAXSIZE];
	size_t n = read(fd, buf, MAXSIZE);
	buf[n] = '\n';
	
	printf("serv:msg from client[%s]. len = %d\n", buf, n);

	if (0 == n) {
		return -1;
	}
	
	printf("serv:will be upper\n");
	
	n = upper(buf, n);
	
	printf("serv:will be send msg to client[%s], len = %d\n", buf, n);
	
	write(fd, buf, n);
	
	printf("serv: send msg to client[%s]. len = %d\n", buf, n);
	
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

	return i;
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
		int nready = epoll_wait(epollfd, &wait_events, 10, -1);
		
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
