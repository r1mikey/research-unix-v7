#ifndef __V7_SYS_DIR_H
#define __V7_SYS_DIR_H

#include "types.h"

#ifndef	DIRSIZ
#define	DIRSIZ	14
#endif

struct	direct
{
	ino_t	d_ino;
	char	d_name[DIRSIZ];
};

#endif
