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
	unsigned char *a = (unsigned char *)sbrk(incr);
	brkend = a + incr;
	return a;
}
