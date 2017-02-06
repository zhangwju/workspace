/***********************************************
* Author: zhangwj
* Filename: test.c 
* Date: 2017-02-06
* Descript: epoll sample
* Warning:
************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h> //epoll

#define EPOLL_LISTEN_CNT 256
#define EPOLL_EVENT_CNT  10
#define EPOLL_LISTEN_TIMEOUT 3

int g_epfd = -1;
int g_listenfd = -1;

int epoll_add_fd(int fd)
{
	struct epoll_event ev;

	memset(&ev, 0, sizeof(ev));
	
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(g_epfd, EPOLL_CTL_ADD, fd, &ev);

	return 0;
}

int init_epoll()
{
	int epfd = -1;
    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));

    epfd = epoll_create(EPOLL_LISTEN_CNT);
    if (epfd < 0) {
		printf("create epoll fd failure\n");
        return -1;
    }
    g_epfd = epfd;

    return 0;
}

int init_listen_sockfd()
{
	int opt = 1;
	int ret = -1;
	int sock = -1;
	struct sockaddr_in addr;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
	    printf("listen socket create error.\n");
	    return -1;
	}
	
	/* 设置socket属性*/
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
	    printf("set opt reuseaddr failed.\n");
	    close(sock);
	    return -1;
	}
	
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret < 0) {
	    printf("set opt reuse port failed.\n");
	    close(sock);
	    return -1;
	}
	
	addr.sin_family = AF_INET;
    addr.sin_port = htons(1900);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if(ret < 0) 
    {
        perror("socket bind fail.\n");
        close(sock);
        return -1;
    }
	
	return sock;
}

int init_epoll_listenfd()
{
	int listenfd = -1;

	listenfd = init_listen_sockfd();
	if (listenfd >= 0) {
		g_listenfd = listenfd;
		epoll_add_fd(listenfd);
	}
	return listenfd;
}

void packet_recv_handle()
{
	int ret = 0;
	char buf[1024] = {0};
	struct sockaddr_in cli_addr;
	socklen_t addrlen;

	addrlen = sizeof(struct sockaddr_in);
	ret = recvfrom(g_listenfd, buf, 1024, 0, (struct sockaddr *)&cli_addr, &addrlen);
	if (ret < 0) {
		printf("recvfrom failure\n");
		return;
	}
	printf("buf: %s\n", buf);
}

void epoll_event_handle()
{
	int fd_cnt = 0;
    int i = 0;
    int fd = -1;
    struct epoll_event events[EPOLL_EVENT_CNT];

	printf("enter event handle..\n"); 
    memset(events, 0, sizeof(events));
    while (1) {
        //等待epoll事件的发生
        fd_cnt = epoll_wait(g_epfd, events, EPOLL_EVENT_CNT, EPOLL_LISTEN_TIMEOUT);
        for (i = 0; i < fd_cnt; i++) {   
            fd = events[i].data.fd;
            if(events[i].events & EPOLLIN) {
                if (g_listenfd == fd) {
					packet_recv_handle();
                }
            }
        }
    }
    
    return;
}

int main(int argc, char **argv)
{
	if (init_epoll()) {
		printf("init epoll failure.\n");
		return -1;
	}

	if (init_epoll_listenfd() < 0) {
		printf("init listenfd failure.\n");
		return -1;
	}
	
	epoll_event_handle();

	return 0;
}
