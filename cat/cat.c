#include <helpers.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    char buf[4096];
    ssize_t nread = 0;
    ssize_t nwrite = 0;

    do
    {
	nread = read_(STDIN_FILENO, buf, sizeof(buf));
	if (nread == -1)
	{
	    fprintf(stderr, "%s\n", strerror(errno));
	}
	nwrite = write_(STDOUT_FILENO, buf, nread);
	if (nwrite < nread)
	{
	    fprintf(stderr, "%s\n", "Unexpected EOF");
	}
    } while (nread == sizeof(buf));
    return 0;
}
