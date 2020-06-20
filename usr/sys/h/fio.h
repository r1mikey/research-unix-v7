#ifndef __V7_SYS_FIO_H
#define __V7_SYS_FIO_H

#include "inode.h"
#include "file.h"

extern int access(struct inode *ip, int mode);
extern int suser(void);
extern int ufalloc(void);
extern struct file * falloc(void);
extern struct file * getf(int f);
extern void closef(struct file *fp);
extern void openi(struct inode *ip, int rw);
extern struct inode * owner(void);

#endif
