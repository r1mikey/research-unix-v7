#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include "defs.h"

extern void *sbrk(int incr);

unsigned char *
setbrk(int incr)
{
	unsigned char *a = sbrk(incr);
	if (a != (unsigned char *)-1) {
		brkend = a + incr;
	}
	return a;
}
