#ifndef __V7_SYS_BIO_H
#define __V7_SYS_BIO_H

#include "types.h"
#include "buf.h"

extern struct buf * bread(dev_t dev, daddr_t blkno);
extern struct buf * breada(dev_t dev, daddr_t blkno, daddr_t rablkno);
extern struct buf * getblk(dev_t dev, daddr_t blkno);
extern void bawrite(struct buf *bp);
extern void bdwrite(struct buf *bp);
extern void bflush(dev_t dev);
extern void brelse(struct buf *bp);
extern void bwrite(struct buf *bp);
extern void clrbuf(struct buf *bp);
extern void swap(int blkno, int coreaddr, int count, int rdflg);
extern struct buf * geteblk(void);

#endif
