#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
struct blk *stakbsy;

unsigned char *stakbas;
unsigned char *brkend;
unsigned char *staktop;
unsigned char *stakbot = nullstr;

/* ========	storage allocation	======== */

unsigned char *
getstak(int asize)
{
	/* allocate requested stack */
	unsigned char *oldstak;
	int size;

	size = round(asize, BYTESPERWORD);
	oldstak = stakbot;
	staktop = stakbot += size;
	return (oldstak);
}

unsigned char *
locstak()
{
	/* set up stack for local use
	 * should be followed by `endstak'
	 */
	if (brkend - stakbot < BRKINCR) {
		setbrk(brkincr);
		if (brkincr < BRKMAX) {
			brkincr += 256;
		}
	}
	return (stakbot);
}

unsigned char *
savstak()
{
	assert(staktop == stakbot);
	return (stakbot);
}

unsigned char *
endstak(unsigned char *argp)
{
	/* tidy up after `locstak' */
	unsigned char *oldstak;
	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (unsigned char *)round(argp, BYTESPERWORD);
	return oldstak;
}

void
tdystak(unsigned char *x)
{
	/* try to bring stack back to x */
	while (ADR(stakbsy) > ADR(x)) {
		free(stakbsy);
		stakbsy = stakbsy->word;
	}
	staktop = stakbot = max(ADR(x), ADR(stakbas));
	rmtemp(x);
}

void
stakchk()
{
	if ((brkend - stakbas) > BRKINCR + BRKINCR) {
		setbrk(-BRKINCR);
	}
}

unsigned char *
cpystak(unsigned char *x)
{
	return (endstak(movstr(x, locstak())));
}
