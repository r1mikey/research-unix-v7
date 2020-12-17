#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

static BOOL chkid(char *nam);
static void namwalk(struct namnod *np);

NAMNOD ps2nod = { NIL, NIL, ps2name }, fngnod = { NIL, NIL, fngname },
       pathnod = { NIL, NIL, pathname }, ifsnod = { NIL, NIL, ifsname },
       ps1nod = { &pathnod, &ps2nod, ps1name },
       homenod = { &fngnod, &ifsnod, homename },
       mailnod = { &homenod, &ps1nod, mailname };

struct namnod *namep = &mailnod;

/* ========	variable and string handling	======== */

int
syslook(char *w, struct sysnod syswds[])
{
	char first;
	char *s;
	struct sysnod *syscan;

	syscan = syswds;
	first = *w;

	while (s = syscan->sysnam) {
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
		char *s = mactrim(arg->argval);
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
setname(char *argi, int xp)
{
	char *argscan = argi;
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
replace(char **a, char *v)
{
	free(*a);
	*a = make(v);
}

void
dfault(struct namnod *n, char *v)
{
	if (n->namval == 0) {
		assign(n, v);
	}
}

void
assign(struct namnod *n, char *v)
{
	if (n->namflg & N_RDONLY) {
		failed(n->namid, wtfailed);
	} else {
		replace(&n->namval, v);
	}
}

int
readvar(char **names)
{
	struct fileblk fb;
	struct fileblk *f = &fb;
	char c;
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
		assign(n, nullstr);
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
assnum(char **p, int i)
{
	itos(i);
	replace(p, numbuf);
}

char *
make(char *v)
{
	char *p;

	if (v) {
		movstr(v, p = (char *)alloc(length(v)));
		return (p);
	} else {
		return (0);
	}
}

struct namnod *
lookup(char *nam)
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
chkid(char *nam)
{
	char *cp = nam;

	if (!letter(*cp)) {
		return (FALSE);
	} else {
		while (*++cp) {
			if (!alphanum(*cp)) {
				return (FALSE);
			};
		};
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
	char *s;

	sigchk();
	if (s = n->namval) {
		prs(n->namid);
		prc('=');
		prs(s);
		newline();
	}
}

static char *
staknam(struct namnod *n)
{
	char *p;

	p = movstr(n->namid, staktop);
	p = movstr("=", p);
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
getenv()
{
	char **e = environ;

	while (*e) {
		setname(*e++, N_ENVNAM);
	}
}

static int namec;

void
countnam(struct namnod *n)
{
	namec++;
}

static char **argnam;

void
pushnam(struct namnod *n)
{
	if (n->namval) {
		*argnam++ = staknam(n);
	}
}

char **
setenv()
{
	char **er;

	namec = 0;
	namscan(countnam);
	argnam = er = (char *)getstak(namec * BYTESPERWORD + BYTESPERWORD);
	namscan(pushnam);
	*argnam++ = 0;
	return (er);
}
