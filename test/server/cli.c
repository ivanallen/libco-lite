#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define PORT 9999

void str_cli(int fd) {
	char buf[100];
	while (1) {
		printf("send to:");
		scanf("%s", buf);
		write(fd, buf, strlen(buf));

		read(fd, buf, 10);
		printf("from server: %s\n", buf);
	}
}



int main () {
	int sockfd;
	struct sockaddr_in servaddr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

	connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	str_cli(sockfd);

	return 0;
}



