#ifndef __V7_SYS_MAP_H
#define __V7_SYS_MAP_H

#include "types.h"

struct map
{
	__s16	m_size;
	__u16	m_addr;
};

extern struct map coremap[];	/* space for core allocation */
extern struct map swapmap[];	/* space for swap allocation */

#endif
