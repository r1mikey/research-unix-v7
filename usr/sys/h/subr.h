#ifndef __V7_SYS_SUBR_H
#define __V7_SYS_SUBR_H

#include "types.h"

struct inode;

extern daddr_t bmap(struct inode *ip, daddr_t bn, int rwflg);
extern int cpass(void);
extern int passc(int c);
extern void bcopy(caddr_t from, caddr_t to, int count);

#endif
