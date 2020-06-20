#ifndef __V7_SYS_REG_H
#define __V7_SYS_REG_H

/*
 * Location of the users' stored
 * registers relative to R0.
 * Usage is u.u_ar0[XX].
 */
#define	R0	(0)
#define	R1	(1)
#define	R2	(2)
#define	R3	(3)
#define	R4	(4)
#define	R5	(5)
#define	R6	(6)
#define	R7	(7)
#define	R8	(8)
#define	R9	(9)
#define	R10	(10)
#define	R11	(11)
#define	R12	(12)
#define	R13	(13)
#define	R14	(14)
#define	R15	(15)
#define FP      (11)
#define	SP	(13)
#define	LR	(14)
#define	PC	(15)
#define	RPS	(16)

#define	TBIT	020		/* PS trace bit */

#endif
