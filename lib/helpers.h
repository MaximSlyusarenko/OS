#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>
#include <stdlib.h>

struct execargs_t
{
	char* program;
	char** args;
	int argc; // arguments count
};

struct execargs_t* exec_new(char* program, char** args, int argc);
ssize_t read_until(int fd, void* buf, ssize_t count, char delimeter);
ssize_t read_(int fd, void* buf, ssize_t count);
ssize_t write_(int fd, const void* buf, ssize_t count);
int spawn(const char* file, char* const argv[]);
int exec(struct execargs_t* args);
int runpiped(struct execargs_t** programs, size_t n);

#endif
