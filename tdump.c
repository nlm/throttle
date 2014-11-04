#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "throttle.h"

void usage()
{
	printf("usage: tdump <statefile>\n");
}

int openstate(const char *statefile)
{
	int fd;
	struct stat stat;

	if ((fd = open(statefile, O_RDONLY)) < 0)
		return -1;

	if ((flock(fd, LOCK_SH)) < 0)
		return -1;

	if (fstat(fd, &stat) < 0)
		return -1;

	if (stat.st_size != 0 && stat.st_size != sizeof(thstate_t))
	{
		printf("not a state file\n");
		exit(101);
	}

	return fd;
}

int closestate(int fd)
{
	return close(fd);
}

thstate_t *readstate(int fd, thstate_t *state)
{
	int rsize;

	lseek(fd, 0, SEEK_SET);
	rsize = read(fd, state, sizeof(thstate_t));

	if (rsize != sizeof(thstate_t))
	{
		return NULL;
	}

	if (strncmp(state->magic, "THRO", 4))
	{
		printf("bad magic in file");
		exit(101);
	}

	return state;
}

void dumpstate(const thstate_t *state)
{
	printf("STATE: startts=%lu, times=%lu\n", (long)state->startts, (long)state->times);
}

int main(int argc, char **argv)
{
	const char *statefile;
	thstate_t thstate;
	thstate_t *state;
	int fd;

	if (argc != 2)
	{
		usage();
		exit(101);
	}

	statefile = argv[1];

	if ((fd = openstate(statefile)) < 0)
	{
		perror("unable to open state file");
		exit(1);
	}

	state = readstate(fd, &thstate);

	if (state == NULL)
		printf("unable to read state\n");
	else
		dumpstate(state);

	closestate(fd);
	return 0;
}
