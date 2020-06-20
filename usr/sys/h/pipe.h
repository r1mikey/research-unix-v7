#ifndef __V7_SYS_PIPE_H
#define __V7_SYS_PIPE_H

#include "inode.h"
#include "file.h"

extern void plock(struct inode *ip);
extern void plock(struct inode *ip);
extern void plock(struct inode *ip);
extern void prele(struct inode *ip);
extern void prele(struct inode *ip);
extern void readp(struct file *fp);
extern void writep(struct file *fp);

#endif
