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

unsigned char *
movstr(const unsigned char *a, unsigned char *b)
{
	while ((*b++ = *a++))
		;
	return (--b);
}

int
any(unsigned char c, const unsigned char *s)
{
	unsigned char d;

	while ((d = *s++)) {
		if (d == c) {
			return (TRUE);
		}
	}
	return (FALSE);
}

int
cf(const unsigned char *s1, const unsigned char *s2)
{
	while (*s1++ == *s2) {
		if (*s2++ == 0) {
			return (0);
		}
	}
	return (*--s1 - *s2);
}

int
length(const unsigned char *as)
{
	const unsigned char *s;

	if ((s = as)) {
		while (*s++)
			;
	}
	return (s - as);
}
