#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"

static void gsort(unsigned char *from[], unsigned char *to[]);
static int split(unsigned char *s);
static unsigned char * execs(unsigned char *ap, unsigned char *t[]);

#define ARGMK 01

extern int errno;
extern const unsigned char *sysmsg[];

/* fault handling */
#define ENOMEM	12
#define ENOEXEC 8
#define E2BIG	7
#define ENOENT	2
#define ETXTBSY 26

/* service routines for `execute' */

void
initio(struct ionod *iop)
{
	unsigned char *ion;
	int iof, fd;

	if (iop) {
		iof = iop->iofile;
		ion = mactrim(iop->ioname);
		if (*ion && (flags & noexec) == 0) {
			if (iof & IODOC) {
				subst(chkopen(ion), (fd = tmpfil()));
				close(fd);
				fd = chkopen(tmpout);
				unlink((const char *)tmpout);
			} else if (iof & IOMOV) {
				if (eq(minus, ion)) {
					fd = -1;
					close(iof & IOUFD);
				} else if ((fd = stoi(ion)) >= USERIO) {
					failed(ion, badfile);
				} else {
					fd = dup(fd);
				}
			} else if ((iof & IOPUT) == 0) {
				fd = chkopen(ion);
			} else if (flags & rshflg) {
				failed(ion, restricted);
			} else if (iof & IOAPP && (fd = open((const char *)ion, 1)) >= 0) {
				lseek(fd, 0L, 2);
			} else {
				fd = create(ion);
			}
			if (fd >= 0) {
				rename(fd, iof & IOUFD);
			};
		}
		initio(iop->ionxt);
	}
}

unsigned char *
getpath(unsigned char *s)
{
	unsigned char *path;
	if (any('/', s)) {
		if (flags & rshflg) {
			failed(s, restricted);
		} else {
			return (unsigned char *)(nullstr);
		}
	} else if ((path = pathnod.namval) == 0) {
		return (unsigned char *)(defpath);
	}
	return (cpystak(path));
}

int
pathopen(unsigned char *path, unsigned char *name)
{
	int f;

	do {
		path = catpath(path, name);
	} while ((f = open((const char *)curstak(), 0)) < 0 && path);
	return (f);
}

unsigned char *
catpath(unsigned char *path, unsigned char *name)
{
	/* leaves result on top of stack */
	unsigned char *scanp = path;
	unsigned char *argp = locstak();

	while (*scanp && *scanp != COLON) {
		*argp++ = *scanp++;
	}
	if (scanp != path) {
		*argp++ = '/';
	}
	if (*scanp == COLON) {
		scanp++;
	}
	path = (*scanp ? scanp : 0);
	scanp = name;
	while ((*argp++ = *scanp++))
		;
	return path;
}

static unsigned char *xecmsg;
static unsigned char **xecenv;

void
execa(unsigned char *at[])
{
	unsigned char *path;
	unsigned char **t = at;

	if ((flags & noexec) == 0) {
		xecmsg = (unsigned char *)notfound;
		path = getpath(*t);
		namscan(exname);
		xecenv = setenv();
		while ((path = execs(path, t)))
			;
		failed(*t, xecmsg);
	}
}

static unsigned char *
execs(unsigned char *ap, unsigned char *t[])
{
	unsigned char *p, *prefix;

	prefix = catpath(ap, t[0]);
	trim(p = curstak());

	sigchk();
	execve((const char *)p, (char *const *)&t[0], (char *const *)xecenv);
	switch (errno) {

	case ENOEXEC:
		flags = 0;
		comdiv = 0;
		ioset = 0;
		clearup(); /* remove open files and for loop junk */
		if (input) {
			close(input);
		}
		close(output);
		output = 2;
		input = chkopen(p);

		/* set up new args */
		setargs(t);
		longjmp(subshell, 1);

	case ENOMEM:
		failed(p, toobig);

	case E2BIG:
		failed(p, arglist);

	case ETXTBSY:
		failed(p, txtbsy);

	default:
		xecmsg = (unsigned char *)badexec;
	case ENOENT:
		return (prefix);
	}
}

/* for processes to be waited for */
#define MAXP 20
static int pwlist[MAXP];
static int pwc;

void
postclr(void)
{
	int *pw = pwlist;

	while (pw <= &pwlist[pwc]) {
		*pw++ = 0;
	}
	pwc = 0;
}

void
post(int pcsid)
{
	int *pw = pwlist;

	if (pcsid) {
		while (*pw) {
			pw++;
		}
		if (pwc >= MAXP - 1) {
			pw--;
		} else {
			pwc++;
		}
		*pw = pcsid;
	}
}

void
await(int i)
{
	int rc = 0, wx = 0;
	int w;
	int ipwc = pwc;

	post(i);
	while (pwc) {
		int p;
		int sig;
		int w_hi;

		{
			int *pw = pwlist;
			p = wait(&w);
			while (pw <= &pwlist[ipwc]) {
				if (*pw == p) {
					*pw = 0;
					pwc--;
				} else {
					pw++;
				}
			}
		}

		if (p == -1) {
			continue;
		}

		w_hi = (w >> 8) & LOBYTE;

		if ((sig = w & 0177)) {
			if (sig == 0177) { /* ptrace! return */
				prs((const unsigned char *)"ptrace: ");
				sig = w_hi;
			}
			if (sysmsg[sig]) {
				if (i != p || (flags & prompt) == 0) {
					prp();
					prn(p);
					blank();
				}
				prs(sysmsg[sig]);
				if (w & 0200) {
					prs(coredump);
				}
			}
			newline();
		}

		if (rc == 0) {
			rc = (sig ? sig | SIGFLG : w_hi);
		}
		wx |= w;
	}

	if (wx && flags & errflg) {
		exitsh(rc);
	}
	exitval = rc;
	exitset();
}

extern BOOL nosubst;

void
trim(unsigned char *at)
{
	unsigned char *p;
	unsigned char c;
	unsigned char q = 0;

	if ((p = at)) {
		while ((c = *p)) {
			*p++ = c & STRIP;
			q |= c;
		}
	}
	nosubst = q & QUOTE;
}

unsigned char *
mactrim(unsigned char *s)
{
	unsigned char *t = macro(s);
	trim(t);
	return t;
}

unsigned char **
scan(int argn)
{
	struct argnod *argp = (struct argnod *)(Rcheat(gchain) & ~ARGMK);
	unsigned char **comargn, **comargm;

	comargn = (unsigned char **)getstak(BYTESPERWORD * argn + BYTESPERWORD);
	comargm = comargn += argn;
	*comargn = ENDARGS;

	while (argp) {
		*--comargn = argp->argval;
		if ((argp = argp->argnxt)) {
			trim(*comargn);
		}
		if (argp == 0 || Rcheat(argp) & ARGMK) {
			gsort(comargn, comargm);
			comargm = comargn;
		}
		argp = (struct argnod *)(Rcheat(argp) & ~ARGMK);
	}
	return comargn;
}

static void
gsort(unsigned char *from[], unsigned char *to[])
{
	int k, m, n;
	int i, j;

	if ((n = to - from) <= 1) {
		return;
	}

	for (j = 1; j <= n; j *= 2)
		;

	for (m = 2 * j - 1; m /= 2;) {
		k = n - m;
		for (j = 0; j < k; j++) {
			for (i = j; i >= 0; i -= m) {
				unsigned char **fromi;
				fromi = &from[i];
				if (cf(fromi[m], fromi[0]) > 0) {
					break;
				} else {
					unsigned char *s;
					s = fromi[m];
					fromi[m] = fromi[0];
					fromi[0] = s;
				}
			}
		}
	}
}

/* Argument list generation */

int
getarg(struct comnod *ac)
{
	struct argnod *argp;
	int count = 0;
	struct comnod *c;

	if ((c = ac)) {
		argp = c->comarg;
		while (argp) {
			count += split(macro(argp->argval));
			argp = argp->argnxt;
		}
	}
	return count;
}

static int
split(unsigned char *s)
{
	unsigned char *argp;
	int c;
	int count = 0;
	int x;

	for (;;) {
		sigchk();
		argp = locstak() + BYTESPERWORD;
		while ((c = *s++, !any(c, ifsnod.namval) && c)) {
			*argp++ = c;
		}
		if (argp == staktop + BYTESPERWORD) {
			if (c) {
				continue;
			} else {
				return (count);
			}
		} else if (c == 0) {
			s--;
		}
		if ((c = expand(argptr((argp = endstak(argp)))->argval, 0))) {
			count += c;
		} else { /* assign(&fngnod, argp->argval); */
			makearg(argptr(argp));
			count++;
		}
		x = ((int)gchain) | ARGMK;
		gchain = (struct argnod *)x;
	}
}
