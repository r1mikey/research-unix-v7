#ifndef __SYS_V7_IGET_H
#define __SYS_V7_IGET_H

#include "types.h"
#include "inode.h"

extern struct inode * iget(dev_t dev, ino_t ino);
extern void iput(struct inode *ip);
extern void iput(struct inode *ip);
extern void itrunc(struct inode *ip);
extern void iupdat(struct inode *ip, time_t *ta, time_t *tm);
extern void wdir(struct inode *ip);
extern struct inode * maknode(int mode);

#endif
