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
    while (1)
    {
    	nread = read_(STDIN_FILENO, buf, sizeof(buf));
    	if (nread == 0)
    	{
    		break;
    	}
    	else if (nread < 0)
    	{
    		fprintf(stderr, "%s\n", "Can't write");
    		return -1;
    	}
    	nwrite = write_(STDOUT_FILENO, buf, nread);
    	if (nwrite < 0)
    	{
    		fprintf(stderr, "%s\n", "Can't write");
    		return -1;
    	}
    }

    return 0;
}
