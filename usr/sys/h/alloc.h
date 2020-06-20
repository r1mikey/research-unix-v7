#ifndef __V7_SYS_ALLOC_H
#define __V7_SYS_ALLOC_H

#include "types.h"
#include "buf.h"
#include "filsys.h"
#include "inode.h"

extern struct buf * alloc(dev_t dev);
extern struct filsys * getfs(dev_t dev);
extern struct inode * ialloc(dev_t dev);
extern void free(dev_t dev, daddr_t bno);
extern void ifree(dev_t dev, ino_t ino);
extern void update(void);

#endif
