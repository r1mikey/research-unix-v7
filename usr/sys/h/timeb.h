#ifndef __V7_SYS_TIMEB_H
#define __V7_SYS_TIMEB_H

#include "types.h"

/*
 * Structure returned by ftime system call
 */
struct timeb {
	time_t	time;
	__u16	millitm;
	__s16	timezone;
	__s16	dstflag;
};

#endif
