#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"


/*
 *	storage allocator
 *	(circular first fit strategy)
 */

#define BUSY 01
#define busy(x)	(Rcheat((x)->word)&BUSY)

unsigned int		brkincr=BRKINCR;
BLKPTR		blokp;			/*current search pointer*/
BLKPTR		bloktop=BLK(_end);	/*top of arena (last blok)*/



void *	alloc(nbytes)
	unsigned int		nbytes;
{
	unsigned int		rbytes = round(nbytes+BYTESPERWORD,BYTESPERWORD);

	for(;;){	int		c=0;
		BLKPTR	p = blokp;
		BLKPTR	q;
		do{	if( !busy(p)
			){	while( !busy(q = p->word) ){ p->word = q->word ;}
				if( ADR(q)-ADR(p) >= rbytes
				){	blokp = BLK(ADR(p)+rbytes);
					if( q > blokp
					){	blokp->word = p->word;
					;}
					p->word=BLK(Rcheat(blokp)|BUSY);
					return(ADR(p+1));
				;}
			;}
			q = p; p = BLK(Rcheat(p->word)&~BUSY);
		}while(	p>q || (c++)==0 );
		addblok(rbytes);
	}
}

void	addblok(reqd)
	unsigned int		reqd;
{
	if( stakbas!=staktop
	){	STKPTR	rndstak;
		BLKPTR	blokstak;

		pushstak(0);
		rndstak=(STKPTR)round(staktop,BYTESPERWORD);
		blokstak=BLK(stakbas)-1;
		blokstak->word=stakbsy; stakbsy=blokstak;
		bloktop->word=BLK(Rcheat(rndstak)|BUSY);
		bloktop=BLK(rndstak);
	;}
	reqd += brkincr; reqd &= ~(brkincr-1);
	blokp=bloktop;
	bloktop=bloktop->word=BLK(Rcheat(bloktop)+reqd);
	bloktop->word=BLK(ADR(_end)+1);
	{
	   STKPTR stakadr=STK(bloktop+2);
	   staktop=movstr(stakbot,stakadr);
	   stakbas=stakbot=stakadr;
	}
}

void	free(ap)
	BLKPTR		ap;
{
	BLKPTR	p;
	int		x;

	if( (p=ap) && p<bloktop
	){	--p;
		x = ((int)p->word) & ~BUSY;
		p->word = (BLKPTR)x;
	;}
}

#ifdef DEBUG
chkbptr(ptr)
	BLKPTR	ptr;
{
	int		exf=0;
	BLKPTR	p = _end;
	BLKPTR	q;
	int		us=0, un=0;

	for(;;){
	   q = Rcheat(p->word)&~BUSY;
	   if( p==ptr ){ exf++ ;}
	   if( q<_end || q>bloktop ){ abort(3) ;}
	   if( p==bloktop ){ break ;}
	   if( busy(p)
	   ){ us += q-p;
	   } else { un += q-p;
	   ;}
	   if( p>=q ){ abort(4) ;}
	   p=q;
	}
	if( exf==0 ){ abort(1) ;}
	prn(un); prc(SP); prn(us); prc(NL);
}
#endif
