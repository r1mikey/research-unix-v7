/* compile-time features
   IALLOC: use all blocks given to ialloc, otherwise ignore unordered blocks
   MSTATS: enable statistics 
   debug: enable assertion checking
   longdebug: full arena checks at every transaction
*/
/* #include <stdlib.h> */

#ifdef longdebug
#define debug 1
#endif
#ifdef debug
#include <stdio.h>
#define ASSERT(p) if(!(p))botch(__LINE__);else
static void
botch(int n)
{
	fprintf(stderr,"bad arena, malloc.c line %d\n" ,n);
	abort();
}
#else
#define ASSERT(p)
#endif

/*	C storage allocator
 *	Circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena.
 *	Each block is preceded by a ptr to the (pointer of) 
 *	the next following block.
 *	Blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN.
 *	Pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle.
 *	Gaps in arena are merely noted as busy blocks.
 *	Last block of arena is empty and
 *	has a pointer to first.
 *	Idle blocks are coalesced during space search.
 *
 *	A different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT,
 *	where INT is integer type to which a pointer can be cast.
 *	Space is obtained from sbrk in multiples of BLOCK.
*/
#define INT unsigned int
#define ALIGN unsigned int
#define NALIGN 2
#define WORD sizeof(union store)
#define BLOCK (1<<15)
#define BUSY 1
#define NULL 0
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store {
	      union store *ptr;
	      ALIGN dummy[NALIGN];
	      int calloc;	/*calloc clears an array of integers*/
};

/* alloca should have type union store.
 * The funny business gets it initialized without complaint. */
#define addr(a) (union store*)&a
static	char *alloca = (char*)&alloca + BUSY;	/* initial arena */
static	union store *allocb = addr(alloca);	/*arena base*/
static	union store *allocc = addr(alloca);	/*all prev blocks known busy*/
static	union store *allocp = addr(alloca);	/*search ptr*/
static	union store *alloct = addr(alloca);	/*top cell*/
static	union store *allocx;	/*for benefit of realloc*/
extern	void *sbrk(int);

/* A cache list of frequently-used sizes is maintained. From each
 * cache entry hangs a chain of available blocks.
 * Chains are located by hashing; a block is deleted from chain
 * when another size collides; all blocks are cleared on sbrk.
*/
#define CACHEMAX 50	/* largest block to be cached (in words) */
#define CACHESIZ 13	/* number of entries (prime) */

static union store *cache[CACHESIZ];
static union store *stdmalloc(unsigned int n);
static void stdfree(union store *p);
static draincache(void);
int cached(union store *p);
static int allock(union store *q);
extern void ialloc(void *p, unsigned int n);


#ifdef MSTATS
#define Mstats(e) e
static long nmalloc, nrealloc, nfree;	/* call statistics */
static long walloc, wfree;		/* space statistics */
static long chit, ccoll, cdrain, cavail;	/* cache statistics */
#else
#define Mstats(e)
#endif

void *
malloc(unsigned int nbytes)
{
	register union store *p;
	register unsigned int nw;
	register union store **cp;

	Mstats(nmalloc++);
	nw = (nbytes+WORD+WORD-1)/WORD;
	if(nw<CACHEMAX && nw>=2) {
		cp = &cache[nw%CACHESIZ];
		p = *cp;
		if(p && nw==clearbusy(p->ptr)-p) {
			ASSERT(testbusy(p->ptr));
			*cp = (++p)->ptr;
			Mstats((chit++, cavail--));
			return (char*)p;
		}
	}
	p = stdmalloc(nw);
	Mstats(p && (walloc += nw));
	return p;
}

static union store *
stdmalloc(register unsigned int nw)
{
	register union store *p, *q;
	register unsigned int temp;
	ASSERT(allock(allocp));
	for(; ; ) {	/* done at most thrice */
		p = allocp;
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				allocp = p;
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q>p);
					p->ptr = q->ptr;
				}
				if(q>=p+nw && p+nw>=p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p <= q) {
				ASSERT(p==allocb&&q==alloct);
				if(++temp>1)
					break;
				ASSERT(allock(allocc));
				p = allocc;
			}
		}
		p = (union store *)sbrk(0);
		temp = ((nw+BLOCK/WORD)/(BLOCK/WORD))*(BLOCK/WORD);
		do {
			if(p+temp > p) {	/*check wrap*/
				q = (union store *)sbrk(temp*WORD);
				if((INT)q != -1)
					goto mextend;
			}
			temp -= (temp-nw)/2;
		} while(temp-nw > 1);
		if(draincache())
			continue;
		return NULL;
mextend:
		draincache();
		ialloc(q, temp*WORD);
	}
found:
	allocp += nw;
	if(q>allocp) {
		allocx = allocp->ptr;
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	if(p<=allocc) {
		ASSERT(p==allocc);
		while(testbusy(allocc->ptr)
		     && (q=clearbusy(allocc->ptr))>allocc)
			allocc = q;
	}
	return p+1;
}

void
free(void *ap)
{
	register union store *p = ap, *q;
	register unsigned int nw;
	register union store **cp;

	if(p==NULL)
		return;
	--p;
	ASSERT(allock(p));
	ASSERT(testbusy(p->ptr));
	ASSERT(!cached(p));
	nw = clearbusy(p->ptr) - p;
	Mstats((nfree++, wfree += nw));
	ASSERT(nw>0);
	if(nw<CACHEMAX && nw>=2) {
		cp = &cache[nw%CACHESIZ];
		q = *cp;
		if(!q || nw==clearbusy(q->ptr)-q) {
			p[1].ptr = q;
			*cp = p;
			Mstats(cavail++);
			return;
		} else Mstats(q && ccoll++);
	}
	stdfree(p+1);
}

/*	freeing strategy tuned for LIFO allocation
*/
static void
stdfree(register union store *p)
{
	allocp = --p;
	if(p < allocc)
		allocc = p;
	ASSERT(allock(allocp));
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > allocp);
}

static
draincache(void)
{
	register union store **cp = cache+CACHESIZ;
	register union store *q;
	int anyfreed = 0;
	while(--cp>=cache) {
		while(q = *cp) {
			ASSERT(testbusy(q->ptr));
			ASSERT((clearbusy(q->ptr)-q)%CACHESIZ==cp-cache);
			ASSERT(q>=allocb&&q<=alloct);
			stdfree(++q);
			anyfreed++;
			*cp = q->ptr;
		}
	}
	Mstats((cdrain+=anyfreed, cavail=0));
	return anyfreed;
}

/* ialloc(q, nbytes) inserts a block that did not come
 * from malloc into the arena
 *
 * q points to new block
 * r points to last of new block
 * p points to last cell of arena before new block
 * s points to first cell of arena after new block
*/

void
ialloc(void *qq, unsigned int nbytes)
{
	register union store *p, *q, *s;
	union store *r;

	q = qq;
	r = q + (nbytes/WORD) - 1;
	q->ptr = r;
	if(q > alloct) {
		p = alloct;
		s = allocb;
		alloct = r;
	} else {
#ifdef IALLOC	/* useful only in small address spaces */
		for(p=allocb; ; p=s) {
			s = clearbusy(p->ptr);
			if(s==allocb)
				break;
			ASSERT(s>p);
			if(s>r) {
				if(p<q)
					break;
				else
					ASSERT(p>r);
			}
		}
		if(allocb > q)
			allocb = q;
		if(allocc > q)
			allocc = q;
		allocp = allocc;
#else
		return;
#endif
	}
	p->ptr = q==p+1? q: setbusy(q);
	r->ptr = s==r+1? s: setbusy(s);
	while(testbusy(allocc->ptr))
		allocc = clearbusy(allocc->ptr);
}

/*	realloc(p, nbytes) reallocates a block obtained from malloc()
 *	to have new size nbytes, and old content.
 *	Returns new location, or 0 on failure
*/

void *
realloc(void *pp, unsigned int nbytes)
{
	register union store *p = pp;
	register union store *s, *t;
	register union store *q;
	register unsigned int nw;
	unsigned int onw;

	Mstats(nrealloc++);
	if(p==NULL)
		return malloc(nbytes);
	ASSERT(testbusy(p[-1].ptr));
	ASSERT(allock(p-1));
	stdfree(p);
	onw = p[-1].ptr - p;
	nw = (nbytes+WORD-1)/WORD;
	q = stdmalloc(nw+1);
	if(nw<onw) {
		Mstats(wfree += q?onw-nw:onw+1);
		onw = nw;
	} else
		Mstats(q && (walloc+=nw-onw) || (wfree+=onw+1));
	if(q==NULL || q==p)
		return (char*)q;
	ASSERT(q<p||q>p[-1].ptr);
	for(s=p, t=q; onw--!=0; )
		*t++ = *s++;
	ASSERT(clearbusy(q[-1].ptr)-q==nw);
	if(q<p && q+nw>=p)
		(q+(q+nw-p))->ptr = allocx;
	ASSERT(allock(q-1));
	return (char *)q;
}

static int
allock(union store *q)
{
#ifdef longdebug
	register union store *p, *r;
	register union store **cp;
	int x, y;
	for(cp=cache+CACHESIZ; --cp>=cache; ) {
		if((p= *cp)==0)
			continue;
		x = clearbusy(p->ptr) - p;
		ASSERT(x%CACHESIZ==cp-cache);
		for( ; p; p = p[1].ptr) {
			ASSERT(testbusy(p->ptr));
			ASSERT(clearbusy(p->ptr)-p==x);
		}
	}
	x = 0, y = 0;
	p = allocb;
	for( ; (r=clearbusy(p->ptr)) > p; p=r) {
		if(p==allocc)
			y++;
		ASSERT(y||testbusy(p->ptr));
		if(p==q)
			x++;
	}
	ASSERT(r==allocb);
	ASSERT(x==1||p==q);
	ASSERT(y||p==allocc);
#else
	ASSERT((unsigned)q/WORD*WORD==(unsigned)q);
	ASSERT(q>=allocb&&q<=alloct);
#endif
	return 1;
}

void
mstats(void)
{
#ifdef MSTATS
#ifndef stderr
#include <stdio.h>
#endif
	fprintf(stderr, "Malloc statistics, including overhead bytes\n");
	fprintf(stderr, "Arena: bottom %ld, top %ld\n",
		(long)clearbusy(alloca), (long)alloct);
	fprintf(stderr, "Calls: malloc %ld, realloc %ld, free %ld\n",
		nmalloc, nrealloc, nfree);
	fprintf(stderr, "Bytes: allocated or extended %ld, ",
		walloc*WORD);
	fprintf(stderr, "freed or cut %ld\n", wfree*WORD);
	fprintf(stderr,"Cache: hits %ld, collisions %ld, discards %ld, blocks avail %ld\n",
		chit, ccoll, cdrain, cavail);
#endif
}

#ifdef debug
int
cached(union store *p)
{
	union store *q = cache[(clearbusy(p->ptr)-p)%CACHESIZ];
	for( ; q; q=q[1].ptr)
		ASSERT(p!=q);
	return 0;
}
#endif
