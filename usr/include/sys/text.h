#ifndef __V7_SYS_TEXT_H
#define __V7_SYS_TEXT_H

#include "types.h"
#include "inode.h"

/*
 * Text structure.
 * One allocated per pure
 * procedure on swap device.
 * Manipulated by text.c
 */
struct text
{
	__s16	x_daddr;	/* disk address of segment (relative to swplo) */
	__s16	x_caddr;	/* core address, if loaded */
	__s16	x_size;		/* size (clicks) */
	struct inode *x_iptr;	/* inode of prototype */
	char	x_count;	/* reference count */
	char	x_ccount;	/* number of loaded references */
	char	x_flag;		/* traced, written flags */
};

extern struct text text[];

#define	XTRC	01		/* Text may be written, exclusive use */
#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */

#endif
