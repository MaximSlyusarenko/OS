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
			if (i > 0 && buf[i] == ' ' && buf[i - 1] == ' ')
			{
				sprintf(word, "%d\n", 0);
				write_(STDOUT_FILENO, word, word_length + 1);
			}	
			else if (buf[i] != ' ')
			{
				word_length++;
			}
			else
			{
				if (word_length > 0)
				{
					sprintf(word, "%d\n", (int) word_length);
					write_(STDOUT_FILENO, word, word_length + 1);
				}
				word_length = 0;
			}
		}
	} while (nread > 0);
	if (word_length > 0)
	{
		sprintf(word, "%d\n", (int) word_length);
		write_(STDOUT_FILENO, word, word_length + 1); 
	}
	return 0;
}								
