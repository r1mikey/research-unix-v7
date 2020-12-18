#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include "defs.h"

unsigned char *
setbrk(int incr)
{
	unsigned char *a = sbrk(incr);
	if (a != (unsigned char *)-1) {
		brkend = a + incr;
	}
	return a;
}
