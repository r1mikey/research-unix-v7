#ifndef __V7_SYS_SYSTM_H
#define __V7_SYS_SYSTM_H

#include "types.h"
#include "param.h"
#include "inode.h"
#include "proc.h"

/*
 * Random set of variables
 * used by more than one
 * routine.
 */
extern char	canonb[];	/* buffer for erase and kill (#@) */
extern struct inode *rootdir;		/* pointer to inode of root directory */
extern struct proc *runq;		/* head of linked list of running processes */
extern int	cputype;		/* type of cpu =40, 45, or 70 */
extern int	lbolt;			/* time of day in HZ not in time */
extern time_t	time;			/* time in sec from 1970 */

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
extern int	nblkdev;

/*
 * Number of character switch entries.
 * Set by cinit/tty.c
 */
extern int	nchrdev;

extern int	mpid;			/* generic for unique process id's */
extern char	runin;			/* scheduling flag */
extern char	runout;			/* scheduling flag */
extern char	runrun;			/* scheduling flag */
extern char	curpri;			/* more scheduling */
extern int	maxmem;			/* actual max memory per process */
extern physadr	lks;			/* pointer to clock device */
extern daddr_t	swplo;			/* block number of swap space */
extern int	nswap;			/* size of swap space */
extern int	updlock;		/* lock for sync */
extern daddr_t	rablock;		/* block to be read ahead */
extern	char	regloc[];	/* locs. of saved user registers (trap.c) */
extern char	msgbuf[];	/* saved "printf" characters */
extern dev_t	rootdev;		/* device of the root */
extern dev_t	swapdev;		/* swapping device */
extern dev_t	pipedev;		/* pipe device */
extern	int	icode[];	/* user init code */
extern	int	szicode;	/* its size */

/*
 * Instrumentation
 */
extern int	dk_busy;
extern long	dk_time[];
extern long	dk_numb[];
extern long	dk_wds[];
extern long	tk_nin;
extern long	tk_nout;

/*
 * Structure of the system-entry table
 */
extern struct sysent {
	char	sy_narg;		/* total number of arguments */
	char	sy_nrarg;		/* number of args in registers */
	void	(*sy_call)(void);		/* handler */
} sysent[];

#endif
