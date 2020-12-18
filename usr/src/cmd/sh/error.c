#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

#include <setjmp.h>

unsigned char *exitadr;

/* ========	error handling	======== */

void
exitset(void)
{
	assnum(&exitadr, exitval);
}

void
sigchk()
{
	/* Find out if it is time to go away.
	 * `trapnote' is set to SIGSET when fault is seen and
	 * no trap has been set.
	 */
	if (trapnote & SIGSET) {
		exitsh(SIGFAIL);
	}
}

void
failed(const unsigned char *s1, const unsigned char *s2)
{
	prp();
	prs(s1);
	if (s2) {
		prs((const unsigned char *)colon);
		prs(s2);
	}
	newline();
	exitsh(ERROR);
}

void
error(const unsigned char *s)
{
	failed(s, NIL);
}

void
exitsh(int xno)
{
	/* Arrive here from `FATAL' errors
	 *  a) exit command,
	 *  b) default trap,
	 *  c) fault with no trap set.
	 *
	 * Action is to return to command level or exit.
	 */
	exitval = xno;
	if ((flags & (forked | errflg | ttyflg)) != ttyflg) {
		done();
	} else {
		clearup();
		longjmp(errshell, 1);
	}
}

void
done(void)
{
	unsigned char *t;
	if ((t = trapcom[0])) {
		trapcom[0] = 0; /*should free but not long */
		execexp(t, 0);
	}
	rmtemp(0);
	exit(exitval);
}

void
rmtemp(struct ionod *base)
{
	while (iotemp > base) {
		unlink((const char *)iotemp->ioname);
		iotemp = iotemp->iolst;
	}
}
