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
#include <poll.h>

struct buf_pair
{
	struct buf_t* buf[2];
	char flag[2];
};

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
	errno = 0;
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

int listen1;
int listen2;

void handler(int num)
{
	if (num == SIGCHLD)
	{
		wait(NULL);
		return;
	}
}

struct pollfd fds[256];
struct buf_pair buffers[127];
int fd_next;
int state = 0;

void close_pipe(int firstfd_num, int secondfd_num, int buf_num)
{
	fprintf(stderr, "Client %d disconnected\n", fds[firstfd_num].fd);
	fprintf(stderr, "Client %d disconnected\n", fds[secondfd_num].fd);
	//fprintf(stderr, "First num: %d %d\n", firstfd_num, fd_next - 2 + (firstfd_num % 2));
	//fprintf(stderr, "Second num: %d %d\n", secondfd_num, fd_next - 2 + (secondfd_num % 2));
	close(fds[firstfd_num].fd);
	close(fds[secondfd_num].fd);
	fds[firstfd_num].revents = 0;
	fds[secondfd_num].revents = 0;
	fds[firstfd_num] = fds[fd_next + state - 2 + (firstfd_num) % 2];
	fds[secondfd_num] = fds[fd_next + state - 2 + (secondfd_num) % 2];
	fd_next -= 2;
	buf_free(buffers[buf_num].buf[0]);
	buf_free(buffers[buf_num].buf[1]);
	int buf_num2 = (fd_next - 2) / 2;
	if (buf_num != buf_num2)
	{
		buffers[buf_num].buf[0] = buffers[buf_num2].buf[0];
		buffers[buf_num].buf[1] = buffers[buf_num2].buf[1];
		buffers[buf_num].flag[0] = buffers[buf_num2].flag[0];
		buffers[buf_num].flag[1] = buffers[buf_num2].flag[1];
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
	fds[0].fd = listen1;
	fds[0].events = POLLIN;
	int r = fcntl(fds[0].fd, F_SETFL, fcntl(fds[0].fd, F_GETFL, 0) | O_NONBLOCK);
	if (r < 0)
	{
		close(fds[0].fd);
		return -1;
	}
	listen2 = port_listen(port2);
	if (listen2 < 0)
	{
		close(fds[0].fd);
		return -1;
	}
	fds[1].fd = listen2;
	fds[1].events = POLLIN;
	r = fcntl(fds[1].fd, F_SETFL, fcntl(fds[1].fd, F_GETFL, 0) | O_NONBLOCK);
	if (r < 0)
	{
		close(fds[0].fd);
		close(fds[1].fd);
		return -1;
	}
	fd_next = 2;
	state = 0;
	int clientfd = -1;
	while (1)
	{
		int num = poll(fds, fd_next, 5000);
		if (num < 0)
		{
			if (errno != EINTR)
			{
				break;
			}
			else
			{
				errno = 0;
				continue;
			}
		}
		else if (num == 0)
		{
			continue;
		}
		for (int i = 0; i < fd_next; i++)
		{
			if (fds[i].revents != 0)
			{
				if (i == state && fd_next < 256) // Can add
				{
					clientfd = get_client(fds[state].fd);
					fprintf(stderr, "Client %d connected\n", clientfd);
					fds[fd_next + state].fd = clientfd;
					fds[fd_next + state].events = POLLIN;
					fds[state].events = 0;
					fds[state ^ 1].events = POLLIN;
					if (state == 1)
					{
						buffers[(fd_next - 2) / 2].buf[0] = buf_new(4096);
						buffers[(fd_next - 2) / 2].buf[1] = buf_new(4096);
						buffers[(fd_next - 2) / 2].flag[0] = 0;
						buffers[(fd_next - 2) / 2].flag[1] = 0;
						fd_next += 2;
					}
					state ^= 1;
				}
				else if (i > 1)
				{
					//fprintf(stderr, "I: %d\n", i);
					int buf_num = (i - (i % 2) - 2) / 2;
					int secondfd_num;
					if (i % 2 == 0)
					{
						secondfd_num = i + 1;
					}
					else
					{
						secondfd_num = i - 1;
					}
					if (fds[i].revents & POLLIN)
					{
						//fprintf(stderr, "fd: %d\n", fds[i].fd);
						int id = i % 2;
						int start_size = buf_size(buffers[buf_num].buf[id]);
						int nread = buf_fill(fds[i].fd, buffers[buf_num].buf[id], start_size + 1);
						//fprintf(stderr, "nread: %d, start size: %d, errno: %s\n", nread, start_size, strerror(errno));
						if (nread == start_size)
						{
							fds[i].events &= ~POLLIN;
							if (buffers[buf_num].flag[id ^ 1])
							{
								close_pipe(i, secondfd_num, buf_num);
							}
							else
							{
								buffers[buf_num].flag[id] = 1;
							}
							fds[secondfd_num].events |= POLLOUT;
						}
						else if (nread < start_size)
						{
							close_pipe(i, secondfd_num, buf_num);
						}
						else
						{
							if (buf_size(buffers[buf_num].buf[id]) == buf_capacity(buffers[buf_num].buf[id]))
							{
								fds[i].events &= ~POLLIN;
							}
							fds[secondfd_num].events |= POLLOUT;
						}
					}
					else if (fds[i].revents & POLLOUT)
					{
						int id = (i % 2) ^ 1;
						int nwrite = buf_flush(fds[i].fd, buffers[buf_num].buf[id], 1);
						if (nwrite < 0)
						{
							if (errno == EAGAIN)
							{
								errno = 0;
								continue;
							}
							close_pipe(i, secondfd_num, buf_num);
						}
						if (buf_size(buffers[buf_num].buf[id]) == 0)
						{
							fds[i].events &= ~POLLOUT;
							if (buffers[buf_num].flag[id])
							{
								shutdown(fds[i].fd, SHUT_WR);
								shutdown(fds[secondfd_num].fd, SHUT_RD);
							}
						}
						if (buf_size(buffers[buf_num].buf[id]) < buf_capacity(buffers[buf_num].buf[id]))
						{
							fds[secondfd_num].events |= POLLIN;
						}
					}
				}
			}
		}
	}
}