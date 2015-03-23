#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char word[4096];
ssize_t word_length = 0;

void reverse()
{
	int i;
	for (i = 0; i < word_length / 2; i++)
	{
		char tmp = word[i];
		word[i] = word[word_length - i - 1];
		word[word_length - i - 1] = tmp;
	}
}		

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
			if (buf[i] != ' ')
			{
				word[word_length] = buf[i];
				word_length++;
			}
			else
			{
				if (word_length > 0)
				{
					reverse();
					write_(STDOUT_FILENO, word, word_length);
					char c[1];
					c[0] = ' ';
					write_(STDOUT_FILENO, c, 1);
				}
				word_length = 0;
			}
		}
	} while (nread > 0);
	if (word_length > 0)
	{
		reverse();
		write_(STDOUT_FILENO, word, word_length);
	}
	return 0;
}								
