#define _POSIX_SOURCE

#include <bufio.h>
#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

struct execargs_t* programs[200];
size_t programsc = 0;

void parse_line(char* line, ssize_t line_size)
{
	char program[4096];
	char* args[200];
	for (int i = 0; i < 200; i++)
	{
		args[i] = malloc(4096);
	}
	int argc = 0;
	char was_prog = 0;
	size_t size = 0;
	int start = 0;
	programsc = 0;
	while (line[start] == ' ')
	{
		start++;
	}
	for (int i = start; i < line_size; i++)
	{
		if (line[i] == '|')
		{
			struct execargs_t* exec = exec_new(program, args, argc);
			programs[programsc] = exec;
			programsc++;
			if (!was_prog) // not space before '|'
			{
				program[size] = 0;
				args[argc][size] = 0;
				argc++;
			}
			else
			{
				args[argc][size] = 0;
				argc++;
			}
			while (line[i + 1] == ' ')
			{
				i++;
			}	
			was_prog = 0;
			argc = 0;
			size = 0;
		}
		else if (line[i] == ' ' || line[i] == '\n' || line[i] == 0)
		{
			if (!was_prog)
			{
				program[size] = 0;
				args[argc][size] = 0;
				argc++;
				was_prog = 1;
			}
			else
			{
				args[argc][size] = 0;
				argc++;
			}
			size = 0;
			while (line[i + 1] == ' ')
			{
				i++;
			}
		}
		else
		{
			if (!was_prog)
			{
				program[size] = line[i];
				if (argc == 0)
				{
					args[argc][size] = line[i];
				}
				size++;
			}
			else
			{
				args[argc][size] = line[i];
				size++;
			}
		}
	}
	struct execargs_t* exec = exec_new(program, args, argc);
	programs[programsc] = exec;
	programsc++;
}

int main()
{
	struct buf_t* buf = buf_new(4096);
	char* line;
	if (buf == NULL)
	{
		fprintf(stderr, "Can't allocate memory for a new buffer");
		return -1;
	}
	ssize_t nread = 0;
	while (1)
	{
		line = (char*) malloc(8096);
		ssize_t nwrite = write(STDOUT_FILENO, "$", 1);
		if (nwrite < 0)
		{
			return -1;
		}
		nread = buf_getline(STDIN_FILENO, buf, line);
		if (nread < 0)
		{
			fprintf(stderr, "%s\n", "Can't read");
			return -1;
		}
		else if (nread == 0)
		{
			break;
		}
		parse_line(line, nread);
		int result = runpiped(programs, programsc);
		if (result < 0)
		{
			return -1;
		}
		if (result < 0)
		{
			nwrite = write(STDOUT_FILENO, "\n", 1);
			if (nwrite < 0)
			{
				return -1;
			}
		}
	}
	buf_free(buf);
	return 0;
}
