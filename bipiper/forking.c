#define _XOPEN_SOURCE 600

#include "../lib/bufio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

int port_listen(char* port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
    hints.ai_next = NULL;
	struct addrinfo* res;
	int result = getaddrinfo(NULL, port, &hints, &res);
	if (result != 0)
	{
		return -1;
	}
	int sockfd;
	struct addrinfo* iterator;
	for (iterator = res; iterator != NULL; iterator = iterator -> ai_next)
	{
		sockfd = socket(iterator -> ai_family, iterator -> ai_socktype, iterator -> ai_protocol);
		if (sockfd < 0)
		{
			continue;
		}
		int one = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
		{
			close(sockfd);
			continue;
		}
		if (bind(sockfd, iterator -> ai_addr, iterator -> ai_addrlen) < 0)
		{
			close(sockfd);
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	if (iterator == NULL)
	{
		return -1;
	}
	if (listen(sockfd, -1) < 0)
	{
		return -1;
	}
	return sockfd;
}

int get_client(int fd)
{
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	int acceptfd;
	while(1)
	{
		acceptfd = accept(fd, (struct sockaddr*) &client, &len);
		if (accept < 0)
		{
			if (errno != EAGAIN && errno != EINTR)
			{
				close(fd);
				return -1;
			}
			errno = 0;
			continue;
		}
		break;
	}
	return acceptfd;
}

int listen1;
int listen2;

int work(int fd1, int fd2)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		close(fd1);
		close(fd2);
		return -1;
	}
	else if (pid == 0)
	{
		struct buf_t* buf = buf_new(4096);
		if (buf == NULL)
		{
			close(listen1);
			close(listen2);
			close(fd1);
			close(fd2);
			return -1;
		}
		while (1)
		{
			ssize_t nread = buf_fill(fd1, buf, 1);
			if (nread == -1)
			{
				buf_free(buf);
				close(listen1);
				close(listen2);
				close(fd1);
				close(fd2);
				return -1;
			}
			else if (nread == 0) // If ^D then close connection
			{
				buf_free(buf);
				close(listen1);
				close(listen2);
				close(fd1);
				close(fd2);
				exit(0);
			}
			else
			{
				ssize_t nwrite = buf_flush(fd2, buf, buf -> size);
				if (nwrite == -1)
				{
					buf_free(buf);
					close(listen1);
					close(listen2);
					close(fd1);
					close(fd2);
					return -1;
				}
			}
		}
	}
	return pid;
}

void handler(int num)
{
	if (num == SIGCHLD)
	{
		wait(NULL);
		return;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: port1 port2\n");
		return -1;
	}
	char* port = argv[1];
	char* port2 = argv[2];
	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);
	listen1 = port_listen(port);
	if (listen1 < 0)
	{
		return -1;
	}
	listen2 = port_listen(port2);
	if (listen2 < 0)
	{
		close(listen1);
		return -1;
	}
	while (1)
	{
		int acceptfd1 = get_client(listen1);
		if (acceptfd1 < 0)
		{
			close(listen1);
			close(listen2);
			return -1;
		}
		int acceptfd2 = get_client(listen2);
		if (acceptfd2 < 0)
		{
			close(listen1);
			close(listen2);
			close(acceptfd1);
			return -1;
		}
		pid_t work1 = work(acceptfd1, acceptfd2);
		if (work1 < 0)
		{
			exit(0);
		}
		pid_t work2 = work(acceptfd2, acceptfd1);
		if (work2 < 0)
		{
			kill(work1, SIGKILL);
			exit(0);
		}
		close(acceptfd1);
		close(acceptfd2);
	}
	close(listen1);
	close(listen2);
}
