#ifndef __V7_SYS_NAMI_H
#define __V7_SYS_NAMI_H

#include "inode.h"

extern int schar(void);
extern int uchar(void);
extern struct inode * namei(int (*func)(), int flag);

#endif
