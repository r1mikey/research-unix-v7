#ifndef __V7_SYS_MACHDEP_H
#define __V7_SYS_MACHDEP_H

#include "types.h"

/* machdep.c */
extern void startup(void);
extern void clkstart(void);
extern void display(void);
extern void mapfree(void *bp);  /* 11/70 specific, must die */
extern void savfp(void *x);
extern void restfp(void *x);  /* TODO */
extern void sendsig(caddr_t p, int signo);

/* console driver */
extern void putchar(unsigned int c);

/* porting functions */
extern int copyiin(const caddr_t src, caddr_t dst, unsigned int sz);
extern int copyin(const caddr_t src, caddr_t dst, unsigned int sz);
extern int copyiout(const caddr_t src, caddr_t dst, unsigned int sz);
extern int copyout(const caddr_t src, caddr_t dst, unsigned int sz);

extern int fubyte(caddr_t addr);
extern int fuibyte(caddr_t addr);
extern int fuiword(caddr_t addr);
extern int fuword(caddr_t addr);
extern int subyte(caddr_t addr, int v);
extern int suibyte(caddr_t addr, int v);
extern int suiword(caddr_t addr, int v);
extern int suword(caddr_t addr, int v);

extern int save(label_t label);
extern void resume(int new_stack, label_t label);

extern void splx(int s);
extern int spl0(void);
extern int spl1(void);
extern int spl2(void);
extern int spl3(void);
extern int spl4(void);
extern int spl5(void);
extern int spl6(void);
extern int spl7(void);

struct u_prof_s;
extern void addupc(caddr_t pc, struct u_prof_s *p, int t);
extern void clearseg(int a);
extern void copyseg(int from, int to);

extern void idle(void);

/* non-portable */
extern unsigned int translate_va_to_pa(unsigned int a);
extern void __endless_spin(void);
extern void vfp_atexec(void);
extern void vfp_discard(void);

#endif
