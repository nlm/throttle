#ifndef __THROTTLE_H__
# define __THROTTLE_H__

typedef struct thstate_s
{
	char magic[4];
	time_t startts;
	unsigned int times;
} thstate_t;	

#endif
