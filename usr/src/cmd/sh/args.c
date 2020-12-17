#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

struct dolnod *freeargs(struct dolnod *blk);
static struct dolnod *copyargs(unsigned char *from[], int n);
static struct dolnod *dolh;

char flagadr[10];

char flagchar[] = { 'x', 'n', 'v', 't', 's', 'i', 'e', 'r', 'k', 'u', 0 };
int flagval[] = { execpr, noexec, readpr, oneflg, stdflg, intflg,
		  errflg, rshflg, keyflg, setflg, 0 };

/* ========	option handling	======== */

int
options(int argc, char **argv)
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

void
setargs(unsigned char *argi[])
{
	/* count args */
	unsigned char **argp = argi;
	int argn = 0;

	while (*argp++ != (unsigned char *)ENDARGS) {
		argn++;
	}

	/* free old ones unless on for loop chain */
	freeargs(dolh);
	dolh = copyargs(argi, argn); /* sets dolv */
	assnum(&dolladr, dolc = argn - 1);
}

struct dolnod *
freeargs(struct dolnod *blk)
{
	unsigned char **argp;
	struct dolnod *argr = 0;
	struct dolnod *argblk;

	if (argblk = blk) {
		argr = argblk->dolnxt;
		if ((--argblk->doluse) == 0) {
			for (argp = argblk->dolarg;
			     *argp != (unsigned char *)ENDARGS; argp++) {
				free(*argp);
			}
			free(argblk->dolarg);
			free(argblk);
		}
	}
	return argr;
}

static struct dolnod *
copyargs(unsigned char *from[], int n)
{
	struct dolnod *np = (struct dolnod *)alloc(sizeof(struct dolnod));
	unsigned char **fp = from;
	unsigned char **pp;

	np->dolnxt = 0;
	np->doluse = 1; /* use count */
	pp = np->dolarg = (unsigned char **)alloc((n + 1) * sizeof(char *));
	dolv = pp;

	while (n--) {
		*pp++ = make(*fp++);
	}
	*pp++ = ENDARGS;
	return (np);
}

void
clearup()
{
	/* force `for' $* lists to go away */
	while (argfor = freeargs(argfor))
		;

	/* clean up io files */
	while (pop())
		;
}

struct dolnod *
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
