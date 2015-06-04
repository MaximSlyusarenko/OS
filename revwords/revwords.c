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
	char buf[4100];
	ssize_t nread = 0;
	ssize_t all = 0;
	do
	{
		nread = read_until(STDIN_FILENO, buf, sizeof(buf), ' ');
		all += nread;
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
					ssize_t nwrite = write_(STDOUT_FILENO, word, word_length);
					if (nwrite < 0)
					{
						fprintf(stderr, "Can't write\n");
						return -1;
					}
					char c[1];
					c[0] = ' ';
					nwrite = write_(STDOUT_FILENO, c, 1);
					if (nwrite < 0)
					{
						fprintf(stderr, "Can't write\n");
						return -1;
					}
					all -= word_length + 1;
				}
				word_length = 0;
			}
		}
	} while (nread > 0);

	if (word_length > 0)
	{
		while (all > 0)
		{
			int i;
			for (i = 0; i < all; i++)
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
						ssize_t nwrite = write_(STDOUT_FILENO, word, word_length);
						if (nwrite < 0)
						{
							fprintf(stderr, "Can't write\n");
							return -1;
						}
						char c[1];
						c[0] = ' ';
						nwrite = write_(STDOUT_FILENO, c, 1);
						if (nwrite < 0)
						{
							fprintf(stderr, "Can't write\n");
							return -1;
						}
						all -= word_length + 1;
					}
					word_length = 0;
				}
			}
		}
	}
	return 0;
}			