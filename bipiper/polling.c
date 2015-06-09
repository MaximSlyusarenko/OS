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

void close_pipe(int firstfd_num, int secondfd_num, int buf_num)
{
	close(fds[firstfd_num].fd);
	close(fds[secondfd_num].fd);
	fds[firstfd_num] = fds[fd_next - 2 + (firstfd_num) % 2];
	fds[secondfd_num] = fds[fd_next - 2 + (secondfd_num) % 2];
	fd_next -= 2;
	buf_free(buffers[buf_num].buf[0]);
	buf_free(buffers[buf_num].buf[1]);
	int buf_num2 = (fd_next - 2) / 2;
	buffers[buf_num].buf[0] = buffers[buf_num2].buf[0];
	buffers[buf_num].buf[1] = buffers[buf_num2].buf[1];
	buffers[buf_num].flag[0] = buffers[buf_num2].flag[0];
	buffers[buf_num].flag[1] = buffers[buf_num2].flag[1];
	fprintf(stderr, "%d\n", buffers[buf_num].flag[0]);
	fprintf(stderr, "%d\n", buffers[buf_num].flag[1]);
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
	int state = 0;
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
					fds[!state].events = POLLIN;
					if (state == 1)
					{
						buffers[(fd_next - 2) / 2].buf[0] = buf_new(4096);
						buffers[(fd_next - 2) / 2].buf[1] = buf_new(4096);
						fd_next += 2;
					}
					state ^= 1;
				}
				else if (i > 1)
				{
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
						int id;
						if (i % 2 != 0)
						{
							id = 1;
						}
						else
						{
							id = 0;
						}
						int start_size = buf_size(buffers[buf_num].buf[id]);
						int nread = buf_fill(fds[i].fd, buffers[buf_num].buf[id], start_size + 1);
						if (nread == start_size)
						{
							fds[i].events ^= POLLIN;
							if (buffers[buf_num].flag[id ^ 1])
							{
								close_pipe(i, secondfd_num, buf_num);
							}
							else
							{
								buffers[buf_num].flag[id] = 1;
							}
						}
						else if (nread < start_size)
						{
							close_pipe(i, secondfd_num, buf_num);
						}
						else
						{
							if (buf_size(buffers[buf_num].buf[id]) == buf_capacity(buffers[buf_num].buf[id]))
							{
								fds[i].events ^= POLLIN;
							}
							fds[secondfd_num].events |= POLLOUT;
						}
					}
					else if (fds[i].revents & POLLOUT)
					{
						int id;
						if (i % 2 == 0)
						{
							id = 1;
						}
						else
						{
							id = 0;
						}
						int nwrite = buf_flush(fds[i].fd, buffers[buf_num].buf[id], 1);
						if (nwrite < 0)
						{
							close_pipe(i, secondfd_num, buf_num);
						}
						if (buf_size(buffers[buf_num].buf[id]) == 0)
						{
							fds[i].events ^= POLLOUT;
							if (buffers[buf_num].flag[id])
							{
								shutdown(fds[i].fd, SHUT_WR);
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

/*#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include "../lib/bufio.h"

#define MAX_CLIENTS_CNT 254
#define BUF_SIZE 4096

int bind_port(char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res;
    if (getaddrinfo(NULL, port, &hints, &res) != 0) {
        return -1;
    }

    struct addrinfo* rp;
    int sfd;
    for(rp = res; rp != NULL; rp = rp->ai_next ) {
        if ((sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) < 0)
            continue;
        int one = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
            perror("setsockopt");
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(sfd);
    }

    if(rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        exit(1);
    }

    freeaddrinfo(res);

    if (listen(sfd, 10) == -1)
        perror("listen");

    return sfd;
}

int accept_client(int sfd) {
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    int fd = accept(sfd, (struct sockaddr*)&client, &sz);
    if (fd == -1) {
        perror("accept");
        close(sfd);
        return -1;
    }
    return fd;
}

int sfd;
int sfd2;

struct mypipe {
    struct buf_t* buffer1;
    struct buf_t* buffer2;
    int closing;
};

struct pollfd pollfds[2 + MAX_CLIENTS_CNT];
struct mypipe buffs[MAX_CLIENTS_CNT / 2];
int nfds;

void close_pipe(int pfd1, int pfd2, int buf_id) {
    buf_free(buffs[buf_id].buffer1);
    buf_free(buffs[buf_id].buffer2);
    close(pollfds[pfd1].fd);
    close(pollfds[pfd2].fd);
    pollfds[pfd1] = pollfds[nfds - 2 + (pfd1 % 2)];
    pollfds[pfd2] = pollfds[nfds - 2 + (pfd2 % 2)];
    nfds -= 2;
    int last_id = (nfds - 2) / 2;
    buffs[buf_id].buffer1 = buffs[last_id].buffer1;
    buffs[buf_id].buffer2 = buffs[last_id].buffer2;
}

int pipe_receive(int pfd1, int pfd2, int buf_id) {
    struct buf_t* buf;
    if (pfd1 % 2)
        buf = buffs[buf_id].buffer2;
    else
        buf = buffs[buf_id].buffer1;
    int prev_size = buf_size(buf);
    int r = buf_fill(pollfds[pfd1].fd, buf, prev_size + 1);
    if (r == prev_size) { // EOF
        pollfds[pfd1].events &= ~(POLLIN);
        if (buffs[buf_id].closing != -1) {
            close_pipe(pfd1, pfd2, buf_id);
        }
        buffs[buf_id].closing = !(pfd1 % 2);
    } else if(r < prev_size) { // error
        close_pipe(pfd1, pfd2, buf_id);
        return r;
    } else {
        if (buf_size(buf) == buf_capacity(buf)) {
            pollfds[pfd1].events &= ~(POLLIN);
        }
        pollfds[pfd2].events |= POLLOUT;
    }
    return 0;
}

int pipe_send(int pfd1, int pfd2, int buf_id) {
    struct buf_t* buf;
    if (pfd1 % 2)
        buf = buffs[buf_id].buffer1;
    else
        buf = buffs[buf_id].buffer2;
    int w = buf_flush(pollfds[pfd1].fd, buf, 1);
    if (w < 0) {
        close_pipe(pfd1, pfd2, buf_id);
        return -1;
    }
    if (buf_size(buf) == 0) {
        pollfds[pfd1].events &= ~(POLLOUT);
        if(buffs[buf_id].closing == (pfd1 % 2)) {

            shutdown(pollfds[pfd1].fd, SHUT_WR);
        }
    }

    if (buf_size(buf) < buf_capacity(buf))
        pollfds[pfd2].events |= POLLIN;

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: polling port1 port2");
        return 1;
    }

    sfd = bind_port(argv[1]);
    sfd2 = bind_port(argv[2]);
    pollfds[0].fd = sfd;
    pollfds[1].fd = sfd2;
    pollfds[0].events = POLLIN;
    nfds = 2;
    int state = 0;
    while (1) {
        int cnt = poll(pollfds, nfds, 10000);
        if (cnt < 0) {
            perror("poll");
            if (errno != EINTR)
                exit(1);
            continue;
        } else if (cnt > 0) {
            for (int i = 0; i < nfds; ++i) {
                if(pollfds[i].revents != 0) {
                    if (i == state && (nfds < 2 + MAX_CLIENTS_CNT)) {
                        int clientfd = accept_client(pollfds[state].fd);
                        fprintf(stderr, "accept client %d\n", clientfd);
                        pollfds[nfds + state].fd = clientfd;
                        pollfds[nfds + state].events = POLLIN;
                        pollfds[state].events = 0;
                        pollfds[!state].events = POLLIN;
                        if(state == 1) {
                            buffs[(nfds - 2) / 2].buffer1 = buf_new(BUF_SIZE);
                            buffs[(nfds - 2) / 2].buffer2 = buf_new(BUF_SIZE);
                            buffs[(nfds - 2) / 2].closing = -1;
                            nfds += 2;
                        }

                        state = !state;
                    } else if(i > 1) {
                        int buf_id = (i - (i % 2) - 2) / 2;
                        int pair = ((i % 2) ? i - 1 : i + 1);
                        if(pollfds[i].revents & POLLIN) {
                            if (pipe_receive(i, pair, buf_id) < 0 ) {
                                fprintf(stderr, "error while receiving, some clients had closed");
                            }
                        } else if (pollfds[i].revents & POLLOUT) {
                            if (pipe_send(i, pair, buf_id) < 0) {
                                fprintf(stderr, "error while sending, some clients had closed");
                            }
                        } else {
                            fprintf(stderr, "some error occured");
                        }
                    }
                }
            }
        }
    }
    close(sfd);
    close(sfd2);
    return 0;
}*/