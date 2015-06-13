#define _POSIX_SOURCE
#define _GNU_SOURCE
#include <helpers.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>

ssize_t read_until(int fd, void* buf, ssize_t count, char delimeter)
{
    ssize_t add = 0;
    ssize_t nread = 0;
    int found = 0;
    do
    {
		nread = read(fd, buf + add, count);
		if (nread < 0)
		{
	    	return -1;
		}
		int i;
		for (i = 0; i < nread; i++)
		{
	    	if (((char*) buf)[add + i] == delimeter)
	    	{
				found = 1;
				break;
	    	}
		}
		add += nread;
		count -= nread;
    } while (!found && count > 0 && nread > 0);
    return add;
}

ssize_t read_(int fd, void* buf, ssize_t count)
{
	return read_until(fd, buf, count, (char) -1);
}	

ssize_t write_(int fd, const void* buf, ssize_t count)
{
    ssize_t add = 0;
    ssize_t nwrite = 0;
    do
    {
		nwrite = write(fd, buf + add, count);
		if (nwrite == -1)
		{
	    	return -1;
		}
		add += nwrite;
		count -= nwrite;
    } while (count > 0 && nwrite > 0);
    return add;
}

int spawn(const char* file, char* const argv[])
{
	pid_t pid;
	pid = fork();
	if (pid != 0)
	{
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) != 0)
		{
			return WEXITSTATUS(status);
		}
		else
		{
			return -1;
		}
	}
	else
	{
		execvp(file, argv);
		return -1;
	}
}	

char return_zero = 0;

void handler2(int num)
{
	if (num == SIGINT)
	{
		return_zero = 1;
		while (wait(NULL) != -1)
		{

		}
	}
}

int exec(struct execargs_t* args)
{
	args -> args[args -> argc] = NULL;
	args -> argc++;
	pid_t pid = fork();
	if (pid < 0)
	{
		return -1;
	}
	else if (pid == 0)
	{
		execvp(args -> program, args -> args);
		exit(-1);
	}
	else
	{
		return pid;
	}
}

int runpiped(struct execargs_t** programs, size_t n)
{
	struct sigaction newsa;
	newsa.sa_handler = &handler2;
	sigemptyset(&newsa.sa_mask);
	newsa.sa_flags = 0;
	struct sigaction oldsa;
	sigaction(SIGINT, &newsa, &oldsa);
	int (*pipefd)[2] = (int(*)[2]) malloc(sizeof(int[2]) * n - 1);
	int* child = (int*) malloc(sizeof(int*) * n);
	int read = -1;
	int write = 0;
	int in = dup(STDIN_FILENO);
	int out = dup(STDOUT_FILENO);
	for (size_t i = 0; i < n; i++)
	{
		if (i > 0)
		{
			if (dup2(pipefd[read][0], STDIN_FILENO) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);
			}
			if (close(pipefd[read][0]) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);
			}
		}
		if (i < n - 1)
		{
			if (pipe2(pipefd[write], O_CLOEXEC) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);			
			}
			if (dup2(pipefd[write][1], STDOUT_FILENO) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);
			}
			if (close(pipefd[write][1]) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);
			}
		}
		else
		{
			if (dup2(out, STDOUT_FILENO) < 0)
			{
				free(pipefd);
				free(child);
				return (return_zero ? 0 : -1);
			}
		}
		child[i] = exec(programs[i]);
		if (child[i] < 0)
		{
			free(pipefd);
			free(child);
			return (return_zero ? 0 : -1);
		}
		read++;
		write++;
	}
	if (dup2(in, STDIN_FILENO) < 0)
	{
		free(pipefd);
		free(child);
		return (return_zero ? 0 : -1);
	}
	for (size_t count = 0; count < n; count++)
	{
		int tmp = wait(NULL);
		if (tmp < 0)
		{
			free(pipefd);
			free(child);
			return (return_zero ? 0 : -1);
		}
		if (tmp == child[count])
		{
			child[count] = -1;
			for (size_t i = 0; i < count; i++)
			{
				if (child[i] != -1)
				{
					if (kill(child[i], SIGKILL) < 0)
					{
						return (return_zero ? 0 : -1);
					}
				}
			}
		}
	}	
	sigaction(SIGINT, &oldsa, NULL);
	free(pipefd);
	free(child);
	return 0;
}

struct execargs_t* exec_new(char* prog, char** arguments, int argcount)
{
	struct execargs_t* answer = (struct execargs_t*) malloc(sizeof(struct execargs_t));
	if (answer == NULL)
	{
		return answer;
	}
	answer -> program = (char*) malloc(4096);
	if (answer -> program == NULL)
	{
		free(answer);
		return NULL;
	}
	int i = 0;
	while (1)
	{
		answer -> program[i] = prog[i];
		if (prog[i] == 0)
		{
			break;
		}
		i++;
	}
	answer -> args = (char**) malloc(sizeof(char*) * (argcount + 1));
	if (answer -> args == NULL)
	{
		free(answer -> program);
		free(answer);
		return NULL;
	}
	for (int j = 0; j < argcount + 1; j++)
	{
		answer -> args[j] = (char*) malloc(4096);
		if (answer -> args[j] == NULL)
		{
			for (int k = 0; k < j; k++)
			{
				free(answer -> args[k]);
			}
			free(answer -> args);
			free(answer -> program);
			free(answer);
		}
	}
	for (int j = 0; j < argcount; j++)
	{
		int k = 0;
		while (1)
		{
			answer -> args[j][k] = arguments[j][k];
			if (arguments[j][k] == 0)
			{
				break;
			}
			k++;
		}
	}
	answer -> argc = argcount;
	return answer;
}