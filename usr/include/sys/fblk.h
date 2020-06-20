#ifndef __V7_SYS_FBLK_H
#define __V7_SYS_FBLK_H

#include "param.h"
#include "types.h"

struct fblk
{
	__s16    	df_nfree;
	daddr_t	df_free[NICFREE];
};

#endif
