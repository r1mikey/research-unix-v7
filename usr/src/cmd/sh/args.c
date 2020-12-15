#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

DOLPTR freeargs();
static DOLPTR copyargs();
static DOLPTR dolh;

char flagadr[10];

char flagchar[] = { 'x', 'n', 'v', 't', 's', 'i', 'e', 'r', 'k', 'u', 0 };
int flagval[] = { execpr, noexec, readpr, oneflg, stdflg, intflg,
		  errflg, rshflg, keyflg, setflg, 0 };

/* ========	option handling	======== */

int options(argc, argv) char **argv;
int argc;
{
	char *cp;
	char **argp = argv;
	char *flagc;
	char *flagp;

	if (argc > 1 && *argp[1] == '-') {
		cp = argp[1];
		flags &= ~(execpr | readpr);
		while (*++cp) {
			flagc = flagchar;

			while (*flagc && *flagc != *cp) {
				flagc++;
			}
			if (*cp == *flagc) {
				flags |= flagval[flagc - flagchar];
			} else if (*cp == 'c' && argc > 2 && comdiv == 0) {
				comdiv = argp[2];
				argp[1] = argp[0];
				argp++;
				argc--;
			} else {
				failed(argv[1], badopt);
			};
		}
		argp[1] = argp[0];
		argc--;
	}

	/* set up $- */
	flagc = flagchar;
	flagp = flagadr;
	while (*flagc) {
		if (flags & flagval[flagc - flagchar]) {
			*flagp++ = *flagc;
		}
		flagc++;
	}
	*flagp++ = 0;

	return (argc);
}

void setargs(argi) char *argi[];
{
	/* count args */
	char **argp = argi;
	int argn = 0;

	while (Rcheat(*argp++) != ENDARGS) {
		argn++;
	}

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh = copyargs(argi, argn); /* sets dolv */
	assnum(&dolladr, dolc = argn - 1);
}

DOLPTR freeargs(blk) DOLPTR blk;
{
	char *argp;
	DOLPTR argr = 0;
	DOLPTR argblk;

	if (argblk = blk) {
		argr = argblk->dolnxt;
		if ((--argblk->doluse) == 0) {
			for (argp = argblk->dolarg; Rcheat(*argp) != ENDARGS;
			     argp++) {
				free(argp);
			}
			free(argblk);
		};
	}
	return (argr);
}

static DOLPTR copyargs(from, n) char *from[];
{
	DOLPTR np = (DOLPTR)alloc(sizeof(char **) * n + 3 * BYTESPERWORD);
	char **fp = from;
	char **pp;

	np->doluse = 1; /* use count */
	pp = np->dolarg;
	dolv = pp;

	while (n--) {
		*pp++ = make(*fp++);
	}
	*pp++ = ENDARGS;
	return (np);
}

clearup()
{
	/* force `for' $* lists to go away */
	while (argfor = freeargs(argfor))
		;

	/* clean up io files */
	while (pop())
		;
}

DOLPTR
useargs()
{
	if (dolh) {
		dolh->doluse++;
		dolh->dolnxt = argfor;
		return (argfor = dolh);
	} else {
		return (0);
	}
}
