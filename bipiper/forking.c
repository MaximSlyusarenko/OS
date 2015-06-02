#define _XOPEN_SOURCE 600

#include <bufio.h>
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
		else if (bind(sockfd, iterator -> ai_addr, iterator -> ai_addrlen) < 0)
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
	struct sockaddr_storage client;
	socklen_t len = sizeof(client);
	int acceptfd;
	while(1)
	{
		acceptfd = accept(fd, (struct sockaddr*) &client, &len);
		if (accept < 0)
		{
			if (errno != EAGAIN && errno != EINTR)
			{
				return -1;
			}
			errno = 0;
			continue;
		}
		break;
	}
	return acceptfd;
}

int work(int fd1, int fd2)
{
	struct buf_t* buf = buf_new(4096);
	if (buf == NULL)
	{
		close(fd1);
		close(fd2);
		return -1;
	}
	while (1)
	{
		ssize_t nread = buf_fill(fd1, buf, buf -> capacity);
		if (nread == -1)
		{
			close(fd1);
			close(fd2);
			return -1;
		}
		else if (nread == 0)
		{
			break;
		}
		else
		{
			ssize_t nwrite = buf_flush(fd2, buf, buf -> size);
			if (nwrite == -1)
			{
				close(fd1);
				close(fd2);
				return -1;
			}
		}
	}
	close(fd1);
	close(fd2);
	return 0;
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
	int listen1 = port_listen(port);
	if (listen1 < 0)
	{
		return -1;
	}
	int listen2 = port_listen(port2);
	if (listen2 < 0)
	{
		return -1;
	}
	while (1)
	{
		int acceptfd1 = get_client(listen1);
		int acceptfd2 = get_client(listen2);
		pid_t pid1 = fork();
		if (pid1 < 0)
		{
			close(acceptfd1);
			close(acceptfd2);
			return -1;
		}
		else if (pid1 == 0)
		{
			exit(work(acceptfd1, acceptfd2));
		}
		pid_t pid2 = fork();
		if (pid2 < 0)
		{
			kill(pid1, SIGKILL);
			close(acceptfd1);
			close(acceptfd2);
			return -1;
		}
		else if (pid2 == 0)
		{
			exit(work(acceptfd2, acceptfd1));
		}
		close(acceptfd1);
		close(acceptfd2);
	}
}
