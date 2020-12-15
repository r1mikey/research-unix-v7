#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

char *trapcom[MAXTRAP];
BOOL trapflg[MAXTRAP];

/* ========	fault handling routines	   ======== */

void fault(sig) int sig;
{
	int flag;

	signal(sig, fault);
	if (sig == MEMF) {
		if (setbrk(brkincr) == -1) {
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

stdsigs()
{
	ignsig(QUIT);
	getsig(INTR);
	getsig(MEMF);
	getsig(ALARM);
}

ignsig(n)
{
	int s, i;

	if ((s = signal(i = n, 1) & 01) == 0) {
		trapflg[i] |= SIGMOD;
	}
	return (s);
}

getsig(n)
{
	int i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0) {
		signal(i, fault);
	}
}

oldsigs()
{
	int i;
	char *t;

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

clrsig(i) int i;
{
	free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD) {
		signal(i, fault);
		trapflg[i] &= ~SIGMOD;
	}
}

chktrap()
{
	/* check for traps */
	int i = MAXTRAP;
	char *t;

	trapnote &= ~TRAPSET;
	while (--i) {
		if (trapflg[i] & TRAPSET) {
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i]) {
				int savxit = exitval;
				execexp(t, 0);
				exitval = savxit;
				exitset();
			};
		};
	}
}
