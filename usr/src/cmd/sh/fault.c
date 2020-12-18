#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

unsigned char *trapcom[MAXTRAP];
BOOL trapflg[MAXTRAP];

/* ========	fault handling routines	   ======== */

static void
fault(int sig)
{
	int flag;

	signal(sig, fault);
	if (sig == MEMF) {
		if (setbrk(brkincr) == (unsigned char *)-1) {
			error(nospace);
		}
	} else if (sig == ALARM) {
		if (flags & waiting) {
			done();
		}
	} else {
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
	}
}

void
stdsigs(void)
{
	ignsig(QUIT);
	getsig(INTR);
	getsig(MEMF);
	getsig(ALARM);
}

int
ignsig(int n)
{
	int s, i;

	if ((s = ((int)signal((i = n), (sig_t)1)) & 01) == 0) {
		trapflg[i] |= SIGMOD;
	}
	return (s);
}

void
getsig(int n)
{
	int i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0) {
		signal(i, fault);
	}
}

void
oldsigs(void)
{
	int i;
	unsigned char *t;

	i = MAXTRAP;
	while (i--) {
		t = trapcom[i];
		if (t == 0 || *t) {
			clrsig(i);
		}
		trapflg[i] = 0;
	}
	trapnote = 0;
}

void
clrsig(int i)
{
	free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD) {
		signal(i, fault);
		trapflg[i] &= ~SIGMOD;
	}
}

void
chktrap(void)
{
	/* check for traps */
	int i = MAXTRAP;
	unsigned char *t;

	trapnote &= ~TRAPSET;
	while (--i) {
		if (trapflg[i] & TRAPSET) {
			trapflg[i] &= ~TRAPSET;
			if ((t = trapcom[i])) {
				int savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
