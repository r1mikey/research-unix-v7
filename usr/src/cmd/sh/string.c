#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

/* ========	general purpose string handling ======== */

char *movstr(a, b) char *a, *b;
{
	while (*b++ = *a++)
		;
	return (--b);
}

int any(c, s) char c;
char *s;
{
	char d;

	while (d = *s++) {
		if (d == c) {
			return (TRUE);
		};
	}
	return (FALSE);
}

int cf(s1, s2) char *s1, *s2;
{
	while (*s1++ == *s2) {
		if (*s2++ == 0) {
			return (0);
		};
	}
	return (*--s1 - *s2);
}

int length(as) char *as;
{
	char *s;

	if (s = as) {
		while (*s++)
			;
		;
	}
	return (s - as);
}
