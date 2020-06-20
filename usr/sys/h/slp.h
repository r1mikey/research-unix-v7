#ifndef __V7_SYS_SLP_H
#define __V7_SYS_SLP_H

#include "types.h"

struct proc;

extern char setpri(struct proc *pp);
extern int newproc(void);
extern void expand(int newsize);
extern void qswtch(void);
extern void sched(void);
extern void setrun(struct proc *p);
extern void sleep(caddr_t chan, int pri);
extern void swtch(void);
extern void wakeup(caddr_t chan);

#endif
