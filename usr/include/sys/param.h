/*
 * tunable variables
 */
#ifndef __V7_SYS_PARAM_H
#define __V7_SYS_PARAM_H

#define	NBUF	29		/* size of buffer cache */
#define	NINODE	200		/* number of in core inodes */
#define	NFILE	175		/* number of in core file structures */
#define	NMOUNT	8		/* number of mountable file systems */
#define	MAXMEM	129		/* max core per process - * 4096 (so, 512KiB user + 4KiB udot) */
#define	MAXUPRC	25		/* max processes per user */
#define	SSIZE	1		/* initial stack size (*PGSZ bytes) */
#define	SINCR	1		/* increment of stack (*PGSZ bytes) */
#define	NOFILE	20		/* max open files per process */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	CMAPSIZ	50		/* size of core allocation area */
#define	SMAPSIZ	50		/* size of swap allocation area */
#define	NCALL	20		/* max simultaneous time callouts */
#define	NPROC	150		/* max number of processes */
#define	NTEXT	40		/* max number of pure texts */
#define	NCLIST	100		/* max total clist size */
#define	HZ	10		/* Ticks/second of the clock */
#define	TIMEZONE (5*60)		/* Minutes westward from Greenwich */
#define	DSTFLAG	1		/* Daylight Saving Time applies in this locality */
#define	MSGBUFS	128		/* Characters saved from error messages */
#define	NCARGS	5120		/* # characters in exec arglist */

/*
 * priorities
 * probably should not be
 * altered too much
 */

#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	40
#define	PUSER	50

/*
 * signals
 * dont change
 */

#define	NSIG	17
/*
 * No more than 16 signals (1-16) because they are
 * stored in bits in a word.
 */
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQUIT	3	/* quit (FS) */
#define	SIGINS	4	/* illegal instruction */
#define	SIGTRC	5	/* trace or breakpoint */
#define	SIGIOT	6	/* iot */
#define	SIGEMT	7	/* emt */
#define	SIGFPT	8	/* floating exception */
#define	SIGKIL	9	/* kill, uncatchable termination */
#define	SIGBUS	10	/* bus error */
#define	SIGSEG	11	/* segmentation violation */
#define	SIGSYS	12	/* bad system call */
#define	SIGPIPE	13	/* end of pipe */
#define	SIGCLK	14	/* alarm clock */
#define	SIGTRM	15	/* Catchable termination */

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */

#define USERTOP	0x60000000	/* top of user address-space */
#define PGSZ	4096		/* size of an MMU page */
#define PGSHIFT	12		/* shift value for turning a physical address into a page */
#define	NBPW	sizeof(int)	/* number of bytes in an integer (used in iomove, exece, fork) */
#define	BSIZE	512		/* size of secondary block (bytes) (used in quite a few places, mostly buffer related) */
/* BSLOP can be 0 unless you have a TIU/Spider */
#define	BSLOP	2		/* In case some device needs bigger buffers (we could set this to 0) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))  /* (used in tloop, bmap) */
#define	BMASK	0777		/* BSIZE-1 */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */
#define	USIZE	1		/* size of user block (*4096) */
#define	UBASE	0140000		/* abs. addr of user block (unused) */
#if !defined(NULL)
#define	NULL	0		/* meh, should probably remove this */
#endif
#define	CMASK	0		/* default mask for file creation */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	SUPERB	((daddr_t)1)	/* block number of the super block */
#define	DIRSIZ	14		/* max characters per directory */
#define	NICINOD	100		/* number of superblock inodes */
#define	NICFREE	50		/* number of superblock free blocks */
#define	INFSIZE	138		/* size of per-proc info for users (unused) */
#define	CBSIZE	12		/* number of chars in a clist block */
#define	CROUND	017		/* clist rounding: sizeof(int *) + CBSIZE - 1 - XXX: CHECK THIS */

/*
 * Some macros for units conversion
 */
/* FIXME: Core clicks (64 bytes) to segments and vice versa */
#define	ctos(x)	((x+127)/128)
#define stoc(x) ((x)*128)

/* Core clicks (4096 bytes) to disk blocks */
#define ctod(x) ((x)<<2)

/* inumber to disk address */
#define	itod(x)	(daddr_t)((((unsigned)x+15)>>3))

/* inumber to disk offset */
#define	itoo(x)	(int)((x+15)&07)

/* clicks to bytes */
#define	ctob(x)	((x)<<(PGSHIFT))

/* bytes to clicks */
#define btoc(x) ((((unsigned int)(x)+(PGSZ-1))>>(PGSHIFT)))

#include "types.h"

typedef	struct { __s16 r[1]; } *	physadr;  /* dodgy */

/*
 * Machine-dependent bits and macros
 */
#if 0
#define	UMODE	0170000		/* usermode bits */
#define	USERMODE(ps)	((ps & UMODE)==UMODE)
#endif
#define PSR_MODE_MASK   0x1f
#define UMODE   0x10
#define USERMODE(ps)    ((ps & PSR_MODE_MASK)==UMODE)

#define	INTPRI	0340		/* Priority bits */
#define	BASEPRI(ps)	((ps & INTPRI) != 0)

#endif
