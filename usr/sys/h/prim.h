#ifndef __V7_SYS_PRIM_H
#define __V7_SYS_PRIM_H

#define	NOSLEEP	0400
#define	FORCE	01000
#define	NORM	02000
#define	KEEP	04000
#define	CLR	010000

struct clist;

extern void cinit(void);
extern int b_to_q(char *cp, int cc, struct clist *q);
extern int getc(struct clist *p);
extern int putc(int c, struct clist *p);

#endif
