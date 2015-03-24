#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <helpers.h>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Incorrect number of arguments: expected more than 2, found: %d\n", argc);
		return 1;
	}
	ssize_t nread = 0;
	char buf[4100];
	ssize_t current_position = 0;
	char current_argument[4100];
	char** arguments = malloc(sizeof(char*) *(argc + 1));
	int i;
	for (i = 0; i < argc - 1; i++)
	{
		arguments[i] = argv[i + 1];
	}	
	arguments[argc - 1] = current_argument;
	do
	{
		nread = read_until(STDIN_FILENO, buf, sizeof(buf), '\n');
		int i = 0;
		for (i = 0; i < nread; i++)
		{
			if (buf[i] != '\n')
			{
				current_argument[current_position] = buf[i];
				current_position++;
			}
			else
			{
				if (current_position != 0)
				{
					current_argument[current_position] = '\0';
					int res = spawn(arguments[0], arguments);
					if (res == 0)
					{
						current_argument[current_position] = '\n';
						current_position++;
						write_(STDOUT_FILENO, current_argument, current_position);
					}	
				}	
				current_position = 0;	
			}
		}
	} while (nread > 0);	
	if (current_position != 0)
	{
		current_argument[current_position] = '\0';
		int res = spawn(arguments[0], arguments);
		if (res == 0)
		{
			current_argument[current_position] = '\n';
			current_position++;
			write_(STDOUT_FILENO, current_argument, current_position);
		}
	}
	free(arguments);
	return 0;
}				
