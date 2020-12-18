#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

unsigned char numbuf[21];

/* printing and io conversion */

void
newline(void)
{
	prc(NL);
}

void
blank(void)
{
	prc(SP);
}

void
prp(void)
{
	if ((flags & prompt) == 0 && cmdadr) {
		prs(cmdadr);
		prs((unsigned char *)colon);
	}
}

void
prs(const unsigned char *as)
{
	const unsigned char *s;

	if ((s = as)) {
		write(output, s, length(s) - 1);
	}
}

void
prc(unsigned char c)
{
	if (c) {
		write(output, &c, 1);
	}
}

void
prt(long int t)
{
	int hr, min, sec;

	t += 30;
	t /= 60;
	sec = t % 60;
	t /= 60;
	min = t % 60;
	if ((hr = t / 60)) {
		prn(hr);
		prc('h');
	}
	prn(min);
	prc('m');
	prn(sec);
	prc('s');
}

void
prn(int n)
{
	itos(n);
	prs(numbuf);
}

void
itos(int n)
{
	unsigned char *abuf;
	unsigned int a, i;
	int pr, d;
	abuf = numbuf;
	pr = FALSE;
	a = n;
	for (i = 10000; i != 1; i /= 10) {
		if ((pr |= (d = a / i))) {
			*abuf++ = d + '0';
		}
		a %= i;
	}
	*abuf++ = a + '0';
	*abuf++ = 0;
}

int
stoi(const unsigned char *icp)
{
	const unsigned char *cp = icp;
	int r = 0;
	unsigned char c;

	while ((c = *cp, digit(c)) && c && r >= 0) {
		r = r * 10 + c - '0';
		cp++;
	}
	if (r < 0 || cp == icp) {
		failed(icp, (const unsigned char *)badnum);
	}
	return (r);
}
