#include "bufio.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#ifdef DEBUG
	#define check(x) if (x == NULL) abort();
#else
	#define check(x)
#endif	

struct buf_t* buf_new(size_t capacity)
{
	struct buf_t* answer = (struct buf_t*) malloc(sizeof(struct buf_t));
	if (answer == NULL)
	{
		return answer;
	}
	answer -> data = malloc(capacity);
	if (answer -> data == NULL)
	{
		free(answer);
		return NULL;
	}
	answer -> size = 0;
	answer -> capacity = capacity;
	return answer;
}

void buf_free(struct buf_t* buf)
{
	check(buf);
	free(buf -> data);
	free(buf);
}

size_t buf_capacity(struct buf_t* buf)
{
	check(buf);
	return buf -> capacity;
}

size_t buf_size(struct buf_t* buf)
{
	check(buf);
	return buf -> size;
}

ssize_t buf_fill(fd_t fd, struct buf_t* buf, size_t required)
{
	check(buf);
	ssize_t nread = 0;
	while (buf -> size < required && buf -> size < buf -> capacity)
	{
		nread = read(fd, buf -> data + buf -> size, buf -> capacity - buf -> size);
		if (nread < 0)
		{
			return -1; // error information is in errno
		}						
		else if (nread == 0)
		{
			break;
		}
		else
		{
			buf -> size += nread;
		}
	}
	return buf -> size;
}

ssize_t buf_flush(fd_t fd, struct buf_t* buf, size_t required)
{
	check(buf);
	ssize_t nwrite = 0;
	ssize_t all = 0;
	while (buf -> size > 0 && all < required)
	{
		nwrite = write(fd, buf -> data + all, buf -> size);
		if (nwrite < 0)
		{
			int tmp = errno;
			memcpy(buf -> data, buf -> data + all, buf -> size);
			errno = tmp;
			return -1; // error information is in errno
		}
		else
		{
			all += nwrite;		
			buf -> size -= nwrite;
		}
	}
	if (buf -> size > 0)
	{
		memcpy(buf -> data, buf -> data + all, buf -> size);
	}
	return all;
}	

ssize_t buf_getline(fd_t fd, struct buf_t* buf, char* dest)
{
	ssize_t symbols_get = 0;
	ssize_t all = 0;
	ssize_t nread = 0;
	while (1)
	{
		ssize_t res = -1;
		ssize_t size = buf -> size;
		int i = 0;
		for (i = 0; i < size; i++)
		{
			char tmp = ((char*) buf -> data)[i];
			if (tmp == '\n')
			{
				memmove(dest, buf -> data, i);
				memmove(buf -> data, buf -> data + i + 1, buf -> size - i - 1);
				buf -> size -= i + 1;
				res = i + 1 + all - nread;
				return res;
			}
		}	
		if (buf -> size != 0)
		{
			memmove(dest, buf -> data, size);
			memmove(buf -> data , buf -> data + size + 1, buf -> size - size - 1);
			buf -> size = 0;
			dest = dest + size;
		}	
		res = size;	
		symbols_get += buf -> size;	
		nread = buf_fill(fd, buf, 1);
		if (nread == 0)
		{
			break;
		}	
		all += nread;
	}
	return symbols_get;			
}

ssize_t buf_write(fd_t fd, struct buf_t* buf, char* src, size_t len)
{
	while (1)
	{
		ssize_t empty = buf -> capacity - buf -> size;
		if (empty > len)
		{
			empty = len;
		}	
		memmove(buf -> data, src, empty);
		buf -> size += empty;
		ssize_t all = 0;
		ssize_t nwrite = buf_flush(fd, buf, buf -> size);
		if (nwrite < 0)
		{
			return -1;
		}	
		memcpy(buf -> data, buf -> data + all, buf -> size);
		buf -> size = 0;
		len -= empty;
		if (len <= 0)
		{	
			return 0;
		}
	}					
}						
