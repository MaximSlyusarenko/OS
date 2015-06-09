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

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: port filename\n");
		return -1;
	}
	char* port = argv[1];
	char* name = argv[2];
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
	struct sockaddr_storage client;
	socklen_t len = sizeof(client);
	while (1)
	{
		int acceptfd = accept(sockfd, (struct sockaddr*) &client, &len);
		if (accept < 0)
		{
			if (errno != EAGAIN)
			{
				close(sockfd);
				return -1;
			}
			errno = 0;
			continue;
		}
		pid_t pid = fork();
		if (pid < 0)
		{
			close(acceptfd);
			return -1;
		}
		else if (pid == 0)
		{
			int filefd = open(name, O_RDONLY);
			if (filefd < 0)
			{
				close(acceptfd);
				exit(-1);
			}
			struct buf_t* buf = buf_new(4096);
			if (buf == NULL)
			{
				close(acceptfd);
				close(filefd);
				exit(-1);
			}
			while (1)
			{
				ssize_t nread = buf_fill(filefd, buf, buf -> capacity);
				if (nread < 0)
				{
					buf_free(buf);
					close(acceptfd);
					close(filefd);
					exit(-1);
				}
				else if (nread == 0)
				{
					buf_free(buf);
					close(acceptfd);
					close(filefd);
					exit(0);
				}
				ssize_t nwrite = buf_flush(acceptfd, buf, buf -> size);
				if (nwrite < 0)
				{
					buf_free(buf);
					close(acceptfd);
					close(filefd);
					exit(-1);
				}
			}
		}
		else
		{
			close(acceptfd);
		}
	}
}
