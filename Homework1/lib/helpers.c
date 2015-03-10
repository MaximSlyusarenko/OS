#include <helpers.h>

ssize_t read_(int fd, void* buf, ssize_t count)
{
    ssize_t add = 0;
    if (count == 0)
    {
	return read(fd, buf, 0);
    }
    ssize_t nread = 0;
    do
    {
	nread = read(fd, buf + add, count);
	if (nread == -1)
	{
	    return -1;
	}
	int i;
	for (i = 0; i < nread; i++)
	{
	    if (((char*) buf)[add + i] == -1)
	    {
		break;
	    }
	}
	add += nread;
	count -= nread;
    } while (count > 0 && nread > 0);
    return add;
}

ssize_t write_(int fd, const void* buf, ssize_t count)
{
    ssize_t add = 0;
    if (count == 0)
    {
	return write(fd, buf, 0);
    }
    ssize_t nwrite = 0;
    do
    {
	nwrite = write(fd, buf + add, count);
	if (nwrite == -1)
	{
	    return -1;
	}
	add += nwrite;
	count -= nwrite;
    } while (count > 0 && nwrite > 0);
    return add;
}
