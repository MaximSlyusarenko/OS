#include <helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char word[4100];
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
	char space[1];
	space[0] = ' ';
	ssize_t nread = 0;
	do
	{
		nread = read_until(STDIN_FILENO, buf, 4096, ' ');
		if (nread < 0)
		{
			fprintf(stderr, "%s\n", strerror(errno));
			return -1;
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
				ssize_t nwrite;
				if (word_length > 0)
				{
					reverse();
					nwrite = write_(STDOUT_FILENO, word, word_length);
					if (nwrite < 0)
					{
						fprintf(stderr, "Can't write\n");
						return -1;
					}
				}
				nwrite = write_(STDOUT_FILENO, space, 1);
				if (nwrite < 0)
				{
					fprintf(stderr, "Can't write\n");
					return -1;
				}
				word_length = 0;
			}
		}
	} while (nread > 0);

	if (word_length > 0)
	{
		reverse();
		ssize_t nwrite = write_(STDOUT_FILENO, word, word_length);
		if (nwrite < 0)
		{
			fprintf(stderr, "Can't write\n");
			return -1;
		}
	}
	return 0;
}			