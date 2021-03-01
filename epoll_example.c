/* epoll_example.c
 * Adapted from poll_input.c on poll(2)
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define errExit(msg) do{perror(msg); exit(EXIT_FAILURE);}while(0)
#define MAX_BUF 10
#define MAX_EVENTS 2
int main(int argc, char **argv)
{
	struct epoll_event ev, epev[MAX_EVENTS];
	int epfd, num_open_fds;
	if ( argc < 2)
	{
		fprintf(stderr, "Usage: %s FILE...\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	epfd = epoll_create1(0);
	if ( epfd == -1)
		errExit("epoll_create1");
	printf("Epoll fd is %d\n", epfd);
	ev.events = EPOLLIN;
	for (int i = 0; i < argc - 1; i++)
	{
		ev.data.fd = open(argv[i+1], O_RDONLY);
		if (ev.data.fd == -1)
			errExit("open");
		printf("Opened \"%s\" on fd %d\n", argv[i+1], ev.data.fd);
		if ( epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
			errExit("epoll_ctl EPOLL_CTL_ADD");
	}	
	num_open_fds = argc - 1;
	while (num_open_fds > 0)
	{
		int rfds;
		printf("About to epoll()\n");
		rfds = epoll_wait(epfd, epev, MAX_EVENTS, -1);
		if (rfds == -1)
			errExit("epoll_wait");
		printf(" %d FDs ready!\n", rfds);
		for (int i = 0; i < rfds; i++)
		{
			char buf[MAX_BUF];
			int fd = epev[i].data.fd;
			uint32_t events = epev[i].events;
			printf("	fd=%d; events: %s%s%s\n", fd,\
					(events & EPOLLIN) ? "EPOLLIN ": "",
					(events & EPOLLERR) ? "EPOLLERR ": "",
					(events & EPOLLHUP) ? "EPOLLHUP ": "");
			if (events & EPOLLIN)
			{
				ssize_t s = read(fd, buf, sizeof(buf));
				if (s == -1)
					errExit("read");
				printf("		read %zd bytes: %.*s\n",\
						s, (int) s,  buf);
			}
			else
			{
				printf("		closing fd %d\n", fd);
				if ( epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL) == -1)
					errExit("epoll_ctl EPOLL_CTL_DEL");
				if ( close(fd) == -1)
					errExit("close");
				num_open_fds--;
			}
		}
	}
	printf("All file descriptors closed; bye\n");
	exit(EXIT_SUCCESS);
}
