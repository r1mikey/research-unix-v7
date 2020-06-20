#ifndef __V7_SYS_SIG_H
#define __V7_SYS_SIG_H

struct proc;

extern int fsig(struct proc *p);
extern int issig(void);
extern void psignal(struct proc *p, int sig);
extern void signal(int pgrp, int sig);

#endif
