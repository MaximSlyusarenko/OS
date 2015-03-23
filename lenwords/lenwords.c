#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char word[4096];
ssize_t word_length = 0;

int main()
{
	char buf[4096];
	ssize_t nread = 0;
	do
	{
		nread = read_until(STDIN_FILENO, buf, sizeof(buf), ' ');
		if (nread == -1)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}
		int i;
		for (i = 0; i < nread; i++)
		{
			if (buf[i] == ' ' && buf[i - 1] == ' ')
			{
				fprintf(stdout, "%d\n", 0);
			}	
			else if (buf[i] != ' ')
			{
				word_length++;
			}
			else
			{
				if (word_length > 0)
				{
					fprintf(stdout, "%d\n", (int) word_length);
				}
				word_length = 0;
			}
		}
	} while (nread > 0);
	if (word_length > 0)
	{
		fprintf(stdout, "%d\n", (int) word_length);
	}
	return 0;
}								
