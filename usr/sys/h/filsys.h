#ifndef __V7_SYS_FILSYS_H
#define __V7_SYS_FILSYS_H

#include "param.h"
#include "types.h"

/*
 * Structure of the super-block
 */
struct	filsys {
	__u16 s_isize;		/* size in blocks of i-list */
	daddr_t	s_fsize;   	/* size in blocks of entire volume */
	__s16  	s_nfree;   	/* number of addresses in s_free */
	daddr_t	s_free[NICFREE];/* free block list */
	__s16  	s_ninode;  	/* number of i-nodes in s_inode */
	ino_t  	s_inode[NICINOD];/* free i-node list */
	char   	s_flock;   	/* lock during free list manipulation */
	char   	s_ilock;   	/* lock during i-list manipulation */
	char   	s_fmod;    	/* super block modified flag */
	char   	s_ronly;   	/* mounted read-only flag */
	time_t 	s_time;    	/* last super block update */
	/* remainder not maintained by this version of the system */
	daddr_t	s_tfree;   	/* total free blocks*/
	ino_t  	s_tinode;  	/* total free inodes */
	__s16  	s_m;       	/* interleave factor */
	__s16  	s_n;       	/* " " */
	char   	s_fname[6];	/* file system name */
	char   	s_fpack[6];	/* file system pack name */
};

#endif
