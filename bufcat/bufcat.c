#include <bufio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>


int main()
{
	struct buf_t* buf = buf_new(4096);
	if (buf == NULL)
	{
		fprintf(stderr, "Can't allocate memory for a new buffer");
		return 1;
	}
	ssize_t nread = 0;
	do
	{
		nread = buf_fill(STDIN_FILENO, buf, buf_capacity(buf));
		if (nread < 0)
		{
			ssize_t nwrite = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
			if (nwrite < 0)
			{
				fprintf(stderr, "I can't do something with buffer, error: %s\n", strerror(errno));
			}	 
			buf_free(buf);
			return 1;
		}
		else if (nread > 0)
		{
			ssize_t nwrite = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
			if (nwrite < 0)
			{
				fprintf(stderr, "Can't write something from buffer, error: %s\n", strerror(errno));
				buf_free(buf);
				return 1;
			}
		}
	} while (nread > 0);
	if (buf_size(buf) > 0)
	{
		ssize_t nwrite = buf_flush(STDOUT_FILENO, buf, buf_size(buf));
		if (nwrite < 0)
		{
			fprintf(stderr, "Can't write something from buffer, error: %s\n", strerror(errno));
			buf_free(buf);
			return 1;
		}
	}
	buf_free(buf);
	return 0;
}							
