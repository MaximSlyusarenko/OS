#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <helpers.h>
#include <bufio.h>

int main(int argc, char** argv)
{
	struct buf_t* buf = buf_new(4096);
	struct buf_t* output_buf = buf_new(4096);
    if (argc < 2)
    {
        fprintf(stderr, "Incorrect number of arguments: expected more than 2, found: %d\n", argc);
        return 1;
    }
	char cur_buf[4096];
    ssize_t nread = 0;
    ssize_t current_position = 0;
    char current_argument[4096];
    char** arguments = malloc(sizeof(char*) *(argc + 1));
    int i;
    for (i = 0; i < argc - 1; i++)
    {
        arguments[i] = argv[i + 1];
    }
    arguments[argc - 1] = current_argument;
	do
	{
    	nread = buf_getline(STDIN_FILENO, buf, cur_buf);
    	int i = 0;
    	for (i = 0; i < nread; i++)
    	{
        	current_argument[current_position] = cur_buf[i];
        	current_position++;
		}			
    	if (current_position != 0)
    	{
        	current_argument[current_position - 1] = '\0';
        	int res = spawn(arguments[0], arguments);
        	if (res == 0)
        	{
            	current_argument[current_position] = '\n';
            	current_position++;
				ssize_t nwrite = buf_write(STDOUT_FILENO, output_buf, current_argument, current_position);
				if (nwrite < 0)
				{
					fprintf(stderr, "Can't write to stdout \n");
					return 1;
				}	
        	}
    	}
		current_position = 0;
	} while (nread > 0);	
    free(arguments);
	buf_free(buf);
	buf_free(output_buf);
    return 0;
}
