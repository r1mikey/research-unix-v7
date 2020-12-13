#ifndef __V7_SYS_MOUNT_H
#define __V7_SYS_MOUNT_H

#include "types.h"
#include "buf.h"
#include "inode.h"
/* XXX: this needs to be an extern declaration */
#include "param.h"

/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
extern struct	mount
{
	dev_t	m_dev;		/* device mounted */
	struct buf *m_bufp;	/* pointer to superblock */
	struct inode *m_inodp;	/* pointer to mounted on inode */
} mount[];

#endif
