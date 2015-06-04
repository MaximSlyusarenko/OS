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
#include <poll.h>

struct buf_pair
{
	struct buf_t* buf[2];
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

struct pollfd fds[256];
struct buf_pair buffers[127];
int fd_next;

void close_clients(int i)
{
	close(fds[2 * i].fd);
	close(fds[2 * i + 1].fd);
	fds[2 * i].fd = -1;
	fds[2 * i + 1].fd = -1;
	if (fds[2 * i].revents & POLLIN)
	{
		struct buf_t* tmp = buffers[i].buf[0];
		int size = buf_size(tmp);
		ssize_t nread = buf_fill(fds[2 * i].fd, tmp, buf_size(tmp) + 1);
		if (nread <= size)
		{
			close_clients(i);
		}
		if (buf_size(tmp) == buf_capacity(tmp))
		{
			fds[2 * i].revents ^= POLLIN;
		}
		if (buf_size(tmp) > 0)
		{
			fds[2 * i + 1].revents |= POLLOUT;
		}
	}
	if (fds[2 * i].revents & POLLOUT)
	{
		struct buf_t* tmp = buffers[i].buf[1];
		ssize_t nwrite = buf_flush(fds[2 * i].fd, tmp, buf_size(tmp));
		if (nwrite < 0)
		{
			close_clients(i);
		}
		if (buf_size(tmp) == 0)
		{
			fds[i].revents ^= POLLOUT;
		}
	}
	for (int i = 1; i < fd_next; i++)
	{
		if (fds[2 * i].fd < 0)
		{
			fds[2 * i] = fds[2 * fd_next];
			fds[2 * i + 1] = fds[2 * (fd_next - 1)];
			fd_next--;
		}
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
	fds[0].fd = listen1;
	fds[0].events = POLLIN;
	int r = fcntl(fds[0].fd, F_SETFL, fcntl(fds[0].fd, F_GETFL, 0) | O_NONBLOCK);
	if (r < 0)
	{
		return -1;
	}
	int listen2 = port_listen(port2);
	if (listen2 < 0)
	{
		return -1;
	}
	fds[1].fd = listen2;
	fds[1].events = POLLIN;
	r = fcntl(fds[1].fd, F_SETFL, fcntl(fds[1].fd, F_GETFL, 0) | O_NONBLOCK);
	if (r < 0)
	{
		return -1;
	}
	fd_next = 1;
	int state = 0;
	int clientfd = -1;
	while (1)
	{
		int num = poll(fds, fd_next, 5000);
		if (num < 0)
		{
			break;
		}
		else if (num == 0)
		{
			continue;
		}
		if (fds[state].revents & POLLIN)
		{
			int cfd = get_client(fds[state].fd);
			if (state == 0)
			{
				clientfd = cfd;
			}
			else
			{
				buffers[fd_next].buf[0] = buf_new(4096);
				buffers[fd_next].buf[1] = buf_new(4096);
				fds[2 * fd_next].fd = clientfd;
				fds[2 * fd_next].events = POLLIN | POLLHUP;
				fds[2 * fd_next + 1].fd = cfd;
				fds[2 * fd_next + 1].events = POLLIN | POLLHUP;
				fd_next++;
			}
			state ^= 1;
		}
		for (int i = 1; i < fd_next; i++)
		{
			if ((fds[2 * i].revents & POLLHUP) || (fds[2 * i].revents & POLLERR))
			{
				close_clients(i);
			}
		}
	}
}
