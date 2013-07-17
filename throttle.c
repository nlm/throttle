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

/*
** Throttle
**
** RETURN VALUES
**
** 100: command throttled
** 101: syntax error
** 102: system error
** 103: error reading/writing state file
**
**
*/

void usage()
{
	printf("usage: throttle <times> <period> <statefile> [<command> [args]]\n");
}

int openstate(const char *statefile)
{
	int fd;
	struct stat stat;

	if ((fd = open(statefile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
		return -1;

	if ((flock(fd, LOCK_EX)) < 0)
		return -1;

	if (fstat(fd, &stat) < 0)
		return -1;

	if (stat.st_size != 0 && stat.st_size != sizeof(thstate_t))
	{
		printf("not a state file\n");
		exit(103);
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
		exit(103);
	}

	return state;
}

int writestate(int fd, const thstate_t *state)
{
	int wsize;

	lseek(fd, 0, SEEK_SET);
	wsize = write(fd, state, sizeof(thstate_t));

	if (wsize < 0)
		perror("err:");
	
	if (wsize != sizeof(thstate_t))
		return -1;

	return 0;
}

#ifdef DEBUG
void dumpstate(const char *label, const thstate_t *state)
{
	printf("STATE: label=%s startts=%u, times=%u\n", label, state->startts, state->times);
}
#endif

thstate_t *initstate(thstate_t *state)
{
	strncpy(state->magic, "THRO", 4);
	state->startts = time(NULL);
	state->times = 0;
	return state;
}	

int xexec(const int argc, char * const *argv)
{
	char **xargv;

	if (!(xargv = malloc(sizeof(char *) * (argc + 1))))
	{
		perror("error: unable to allocate\n");
		exit(102);
	}
	
	memcpy(xargv, argv, sizeof(char *) * argc);
	xargv[argc] = NULL;

	return execv(xargv[0], xargv);
	free(xargv);
}

int main(int argc, char **argv)
{
	unsigned int times;
	unsigned int period;
	const char *statefile;
	thstate_t thstate;
	thstate_t *state;
	time_t now;
	int fd;

	if (argc < 4)
	{
		usage();
		exit(101);
	}

	times = atoi(argv[1]);
	period = atoi(argv[2]);
	statefile = argv[3];

	if (times < 1 || period < 1)
	{
		usage();
		exit(101);
	}

	if ((fd = openstate(statefile)) < 0)
	{
		perror("unable to open state file");
		exit(103);
	}

	state = readstate(fd, &thstate);

	if (state == NULL)
	{
#ifdef DEBUG
		printf("init\n");
#endif
		state = initstate(&thstate);
#ifdef DEBUG
		dumpstate("after-init", state);
#endif
	}

	now = time(NULL);
#ifdef DEBUG
	dumpstate("before-update", state);
#endif

	/* updating */
	if (state->startts + period > now)
	{
		state->times++;
#ifdef DEBUG
		printf("increase\n");
#endif
	}
	else
	{
		state->startts = now;
		state->times = 1;
#ifdef DEBUG
		printf("renew\n");
#endif
	}

#ifdef DEBUG
	dumpstate("after-update", state);
#endif
	writestate(fd, state);
	closestate(fd);

	/* take action */
	if (state->times <= times)
	{
#ifdef DEBUG
		printf("action: run %s\n", argv[4]);
#endif
		if (xexec(argc - 4, argv + 4) == -1)
		{
			perror("exec");
			return 1;
		}
	}
	else
	{
#ifdef DEBUG
		printf("action: block\n");
#endif
		exit(100);
	}

	return 0;
}
