#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"

DOLPTR freeargs();
static DOLPTR copyargs();
static DOLPTR	dolh;

CHAR	flagadr[10];

CHAR	flagchar[] = {
	'x',	'n',	'v',	't',	's',	'i',	'e',	'r',	'k',	'u',	0
};
INT	flagval[]  = {
	execpr,	noexec,	readpr,	oneflg,	stdflg,	intflg,	errflg,	rshflg,	keyflg,	setflg,	0
};

/* ========	option handling	======== */


INT	options(argc,argv)
	STRING		*argv;
	INT		argc;
{
	STRING	cp;
	STRING	*argp=argv;
	STRING	flagc;
	STRING		flagp;

	if( argc>1 && *argp[1]=='-'
	){	cp=argp[1];
		flags &= ~(execpr|readpr);
		while( *++cp
		){	flagc=flagchar;

			while( *flagc && *flagc != *cp ){ flagc++ ;}
			if( *cp == *flagc
			){	flags |= flagval[flagc-flagchar];
			} else if ( *cp=='c' && argc>2 && comdiv==0
			){	comdiv=argp[2];
				argp[1]=argp[0]; argp++; argc--;
			} else {	failed(argv[1],badopt);
			;}
		;}
		argp[1]=argp[0]; argc--;
	;}

	/* set up $- */
	flagc=flagchar;
	flagp=flagadr;
	while( *flagc
	){ if( flags&flagval[flagc-flagchar]
	   ){ *flagp++ = *flagc;
	   ;}
	   flagc++;
	;}
	*flagp++=0;

	return(argc);
}

void	setargs(argi)
	STRING		argi[];
{
	/* count args */
	STRING	*argp=argi;
	INT		argn=0;

	while( Rcheat(*argp++)!=ENDARGS ){ argn++ ;}

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh=copyargs(argi,argn);	/* sets dolv */
	assnum(&dolladr,dolc=argn-1);
}

DOLPTR freeargs(blk)
	DOLPTR		blk;
{
	STRING	argp;
	DOLPTR	argr=0;
	DOLPTR	argblk;

	if( argblk=blk
	){	argr = argblk->dolnxt;
		if( (--argblk->doluse)==0
		){	for( argp=argblk->dolarg; Rcheat(*argp)!=ENDARGS; argp++
			){ free(argp) ;}
			free(argblk);
		;}
	;}
	return(argr);
}

static DOLPTR	copyargs(from, n)
	STRING		from[];
{
	DOLPTR	np=(DOLPTR)alloc(sizeof(STRING*)*n+3*BYTESPERWORD);
	STRING *	fp=from;
	STRING *	pp;

	np->doluse=1;	/* use count */
	pp = np->dolarg;
	dolv = pp;

	while( n--
	){ *pp++ = make(*fp++) ;}
	*pp++ = ENDARGS;
	return(np);
}

clearup()
{
	/* force `for' $* lists to go away */
	while( argfor=freeargs(argfor) );

	/* clean up io files */
	while( pop() );
}

DOLPTR	useargs()
{
	if( dolh
	){	dolh->doluse++;
		dolh->dolnxt=argfor;
		return(argfor=dolh);
	} else {	return(0);
	;}
}
