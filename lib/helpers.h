#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>

ssize_t read_(int fd, void* buf, ssize_t count);
ssize_t write_(int fd, const void* buf, ssize_t count);

#endif
