#include "../h/param.h"
#include "../h/systm.h"
#include "../h/callo.h"
#include "../h/seg.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/reg.h"
#include "../h/prf.h"
#include "../h/machdep.h"
#include "../h/slp.h"
#include "../h/sig.h"

#define	SCHMAG	8/10

int	lbolt;
time_t time;
struct  callo   callout[NCALL];

long	dk_time[32];
int	dk_busy;

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	lightning bolt wakeup (every second)
 *	alarm clock signals
 *	jab the scheduler
 */

/* unused, but we keep it for now */
physadr	lks;			/* pointer to clock device */


void clock(dev_t dev, unsigned int sp, unsigned int r1, unsigned int nps, unsigned int r0, caddr_t pc, unsigned int ps)
{
	register struct callo *p1, *p2;
	register struct proc *pp;
	int a;
	extern caddr_t waitloc;

#if 0
	/*
	 * restart clock
	 */

	lks->r[0] = 0115;
#endif
	/*
	 * display register
	 */

	display();

	/*
	 * callouts
	 * if none, just continue
	 * else update first non-zero time
	 */

	if(callout[0].c_func == NULL)
		goto out;
	p2 = &callout[0];
	while(p2->c_time<=0 && p2->c_func!=NULL)
		p2++;
	p2->c_time--;

	/*
	 * if ps is high, just return
	 */
#if 0
	if (BASEPRI(ps))
		goto out;
#endif

	/*
	 * callout
	 */

	/* spl5(); */
	if(callout[0].c_time <= 0) {
		p1 = &callout[0];
		while(p1->c_func != 0 && p1->c_time <= 0) {
			(*p1->c_func)(p1->c_arg);
			p1++;
		}
		p2 = &callout[0];
		while(p2->c_func = p1->c_func) {
			p2->c_time = p1->c_time;
			p2->c_arg = p1->c_arg;
			p1++;
			p2++;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */
out:
#if 1
	a = dk_busy&07;
	if (USERMODE(ps)) {
		u.u_utime++;
		if(u.u_prof.pr_scale)
			addupc(pc, &u.u_prof, 1);
		if(u.u_procp->p_nice > NZERO)
			a += 8;
	} else {
		a += 16;
		if (pc >= (waitloc - (4 * 4)) && pc <= waitloc)
			a += 8;
		u.u_stime++;
	}
	dk_time[a] += 1;
#endif
	pp = u.u_procp;
	if(++pp->p_cpu == 0)
		pp->p_cpu--;
	if(++lbolt >= HZ) {
		/* if (BASEPRI(ps)) */
		lbolt -= HZ;
		++time;
		/* spl1(); */
		runrun++;
		wakeup((caddr_t)&lbolt);
		for(pp = &proc[0]; pp < &proc[NPROC]; pp++)
		if (pp->p_stat && pp->p_stat<SZOMB) {
			if(pp->p_time != 127)
				pp->p_time++;
			if(pp->p_clktim)
				if(--pp->p_clktim == 0)
					psignal(pp, SIGCLK);
			a = (pp->p_cpu & 0377)*SCHMAG + pp->p_nice - NZERO;
			if(a < 0)
				a = 0;
			if(a > 255)
				a = 255;
			pp->p_cpu = a;
			if(pp->p_pri >= PUSER)
				setpri(pp);
		}
		if(runin!=0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
	}
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
void timeout(int (*fun)(), caddr_t arg, int tim)
{
	struct callo *p1, *p2;
	int t;
	int s;

	t = tim;
	p1 = &callout[0];
	s = spl7();
	while(p1->c_func != 0 && p1->c_time <= t) {
		t -= p1->c_time;
		p1++;
	}
	if (p1 >= &callout[NCALL-1])
		panic("Timeout table overflow");
	p1->c_time -= t;
	p2 = p1;
	while(p2->c_func != 0)
		p2++;
	while(p2 >= p1) {
		(p2+1)->c_time = p2->c_time;
		(p2+1)->c_func = p2->c_func;
		(p2+1)->c_arg = p2->c_arg;
		p2--;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	splx(s);
}
