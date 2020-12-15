#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

/* A chain of ptrs of stack blocks that
 * have become covered by heap allocation.
 * `tdystak' will return them to the heap.
 */
BLKPTR      stakbsy;

STKPTR      stakbas;
STKPTR      brkend;
STKPTR      staktop;
STKPTR		stakbot=nullstr;



/* ========	storage allocation	======== */

STKPTR	getstak(asize)
	INT		asize;
{	/* allocate requested stack */
	STKPTR	oldstak;
	INT		size;

	size=round(asize,BYTESPERWORD);
	oldstak=stakbot;
	staktop = stakbot += size;
	return(oldstak);
}

STKPTR	locstak()
{	/* set up stack for local use
	 * should be followed by `endstak'
	 */
	if( brkend-stakbot<BRKINCR
	){	setbrk(brkincr);
		if( brkincr < BRKMAX
		){	brkincr += 256;
		;}
	;}
	return(stakbot);
}

STKPTR	savstak()
{
	assert(staktop==stakbot);
	return(stakbot);
}

STKPTR	endstak(argp)
	STRING	argp;
{	/* tidy up after `locstak' */
	STKPTR	oldstak;
	*argp++=0;
	oldstak=stakbot; stakbot=staktop=round(argp,BYTESPERWORD);
	return(oldstak);
}

void	tdystak(x)
	STKPTR 	x;
{
	/* try to bring stack back to x */
	while( ADR(stakbsy)>ADR(x)
	){ free(stakbsy);
	   stakbsy = stakbsy->word;
	;}
	staktop=stakbot=max(ADR(x),ADR(stakbas));
	rmtemp(x);
}

stakchk()
{
	if( (brkend-stakbas)>BRKINCR+BRKINCR
	){	setbrk(-BRKINCR);
	;}
}

STKPTR	cpystak(x)
	STKPTR		x;
{
	return(endstak(movstr(x,locstak())));
}
