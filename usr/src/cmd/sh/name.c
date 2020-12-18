#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

static BOOL chkid(const unsigned char *nam);
static void namwalk(struct namnod *np);

struct namnod ps2nod = { NIL, NIL, (unsigned char *)ps2name }, fngnod = { NIL, NIL, (unsigned char *)fngname },
       pathnod = { NIL, NIL, (unsigned char *)pathname }, ifsnod = { NIL, NIL, (unsigned char *)ifsname },
       ps1nod = { &pathnod, &ps2nod, (unsigned char *)ps1name },
       homenod = { &fngnod, &ifsnod, (unsigned char *)homename },
       mailnod = { &homenod, &ps1nod, (unsigned char *)mailname };

struct namnod *namep = &mailnod;

/* ========	variable and string handling	======== */

int
syslook(const unsigned char *w, const struct sysnod syswds[])
{
	char first;
	const unsigned char *s;
	const struct sysnod *syscan;

	syscan = syswds;
	first = *w;

	while ((s = syscan->sysnam)) {
		if (first == *s && eq(w, s)) {
			return (syscan->sysval);
		}
		syscan++;
	}
	return (0);
}

void
setlist(struct argnod *arg, int xp)
{
	while (arg) {
		unsigned char *s = mactrim(arg->argval);
		setname(s, xp);
		arg = arg->argnxt;
		if (flags & execpr) {
			prs(s);
			if (arg) {
				blank();
			} else {
				newline();
			}
		}
	}
}

void
setname(unsigned char *argi, int xp)
{
	unsigned char *argscan = argi;
	struct namnod *n;

	if (letter(*argscan)) {
		while (alphanum(*argscan)) {
			argscan++;
		}
		if (*argscan == '=') {
			*argscan = 0;
			n = lookup(argi);
			*argscan++ = '=';
			attrib(n, xp);
			if (xp & N_ENVNAM) {
				n->namenv = n->namval = argscan;
			} else {
				assign(n, argscan);
			}
			return;
		};
	}
	failed(argi, notid);
}

void
replace(unsigned char **a, unsigned char *v)
{
	free(*a);
	*a = make(v);
}

void
dfault(struct namnod *n, unsigned char *v)
{
	if (n->namval == 0) {
		assign(n, v);
	}
}

void
assign(struct namnod *n, unsigned char *v)
{
	if (n->namflg & N_RDONLY) {
		failed(n->namid, wtfailed);
	} else {
		replace(&n->namval, v);
	}
}

int
readvar(unsigned char **names)
{
	struct fileblk fb;
	struct fileblk *f = &fb;
	unsigned char c;
	int rc = 0;
	struct namnod *n =
	    lookup(*names++); /* done now to avoid storage mess */
	unsigned char *rel = relstak();

	push(f);
	initf(dup(0));
	if (lseek(0, 0L, 1) == -1) {
		f->fsiz = 1;
	}

	for (;;) {
		c = nextc(0);
		if ((*names && any(c, ifsnod.namval)) || eolchar(c)) {
			zerostak();
			assign(n, absstak(rel));
			setstak(rel);
			if (*names) {
				n = lookup(*names++);
			} else {
				n = 0;
			}
			if (eolchar(c)) {
				break;
			}
		} else {
			pushstak(c);
		}
	}
	while (n) {
		assign(n, (unsigned char *)nullstr);
		if (*names) {
			n = lookup(*names++);
		} else {
			n = 0;
		}
	}

	if (eof) {
		rc = 1;
	}
	lseek(0, (long)(f->fnxt - f->fend), 1);
	pop();
	return (rc);
}

void
assnum(unsigned char **p, int i)
{
	itos(i);
	replace(p, numbuf);
}

unsigned char *
make(const unsigned char *v)
{
	unsigned char *p;

	if (v) {
		movstr(v, p = alloc(length(v)));
		return (p);
	} else {
		return (0);
	}
}

struct namnod *
lookup(const unsigned char *nam)
{
	struct namnod *nscan = namep;
	struct namnod **prev = NIL;
	int LR;

	if (!chkid(nam)) {
		failed(nam, notid);
	}
	while (nscan) {
		if ((LR = cf(nam, nscan->namid)) == 0) {
			return (nscan);
		} else if (LR < 0) {
			prev = &(nscan->namlft);
		} else {
			prev = &(nscan->namrgt);
		}
		nscan = *prev;
	}

	/* add name node */
	nscan = (struct namnod *)alloc(sizeof *nscan);
	nscan->namlft = nscan->namrgt = NIL;
	nscan->namid = make(nam);
	nscan->namval = 0;
	nscan->namflg = N_DEFAULT;
	nscan->namenv = 0;
	return (*prev = nscan);
}

static BOOL
chkid(const unsigned char *nam)
{
	const unsigned char *cp = nam;

	if (!letter(*cp)) {
		return (FALSE);
	} else {
		while (*++cp) {
			if (!alphanum(*cp)) {
				return (FALSE);
			}
		}
	}
	return (TRUE);
}

static void (*namfn)(struct namnod *);
void
namscan(void (*fn)(struct namnod *))
{
	namfn = fn;
	namwalk(namep);
}

static void
namwalk(struct namnod *np)
{
	if (np) {
		namwalk(np->namlft);
		(*namfn)(np);
		namwalk(np->namrgt);
	}
}

void
printnam(struct namnod *n)
{
	unsigned char *s;

	sigchk();
	if ((s = n->namval)) {
		prs(n->namid);
		prc('=');
		prs(s);
		newline();
	}
}

static unsigned char *
staknam(struct namnod *n)
{
	unsigned char *p;

	p = movstr(n->namid, staktop);
	p = movstr((const unsigned char *)"=", p);
	p = movstr(n->namval, p);
	return (getstak(p + 1 - ADR(stakbot)));
}

void
exname(struct namnod *n)
{
	if (n->namflg & N_EXPORT) {
		free(n->namenv);
		n->namenv = make(n->namval);
	} else {
		free(n->namval);
		n->namval = make(n->namenv);
	}
}

void
printflg(struct namnod *n)
{
	if (n->namflg & N_EXPORT) {
		prs(export);
		blank();
	}
	if (n->namflg & N_RDONLY) {
		prs(readonly);
		blank();
	}
	if (n->namflg & (N_EXPORT | N_RDONLY)) {
		prs(n->namid);
		newline();
	}
}

void
getenv(void)
{
	char **e = environ;

	while (*e) {
		setname((unsigned char *)*e++, N_ENVNAM);
	}
}

static int namec;

void
countnam(struct namnod *n)
{
	namec++;
}

static unsigned char **argnam;

void
pushnam(struct namnod *n)
{
	if (n->namval) {
		*argnam++ = staknam(n);
	}
}

unsigned char **
setenv(void)
{
	unsigned char **er;

	namec = 0;
	namscan(countnam);
	argnam = er = (unsigned char **)getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return (er);
}
