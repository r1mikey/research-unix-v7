#ifndef __V7_SYS_RDWRI_H
#define __V7_SYS_RDWRI_H

#include "inode.h"

extern unsigned int max(unsigned int a, unsigned int b);
extern unsigned int min(unsigned int a, unsigned int b);
extern void readi(struct inode *ip);
extern void writei(struct inode *ip);
extern void writei(struct inode *ip);

#endif
