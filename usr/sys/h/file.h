#ifndef __V7_SYS_FILE_H
#define __V7_SYS_FILE_H

#include "types.h"

/*
 * One file structure is allocated
 * for each open/creat/pipe call.
 * Main use is to hold the read/write
 * pointer associated with each open
 * file.
 */
struct	file
{
	char	f_flag;
	char	f_count;	/* reference count */
	struct inode *f_inode;	/* pointer to inode structure */
	off_t	f_offset;	/* read/write character pointer */
};

#ifdef KERNEL
extern struct file file[];	/* The file table itself */
#endif

/* flags */
#define	FREAD	01
#define	FWRITE	02
#define	FPIPE	04
#define	FKERNEL	040

#endif
