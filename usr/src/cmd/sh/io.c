#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "dup.h"

int ioset;

/* ========	input output and file copying ======== */

void
initf(int fd)
{
	struct fileblk *f = standin;

	f->fdes = fd;
	f->fsiz = ((flags & (oneflg | ttyflg)) == 0 ? BUFSIZ : 1);
	f->fnxt = f->fend = f->fbuf;
	f->feval = 0;
	f->flin = 1;
	f->feof = FALSE;
}

int
estabf(unsigned char *s)
{
	struct fileblk *f;

	(f = standin)->fdes = -1;
	f->fend = length(s) + (f->fnxt = s);
	f->flin = 1;
	return (f->feof = (s == 0));
}

void
push(struct fileblk *af)
{
	struct fileblk *f;

	(f = af)->fstak = standin;
	f->feof = 0;
	f->feval = 0;
	standin = f;
}

BOOL
pop(void)
{
	struct fileblk *f;

	if ((f = standin)->fstak) {
		if (f->fdes >= 0) {
			close(f->fdes);
		}
		standin = f->fstak;
		return (TRUE);
	} else {
		return (FALSE);
	}
}

void
chkpipe(int *pv)
{
	if (pipe(pv) < 0 || pv[INPIPE] < 0 || pv[OTPIPE] < 0) {
		error(piperr);
	}
}

int
chkopen(const unsigned char *idf)
{
	int rc;

	if ((rc = open((const char *)idf, 0)) < 0) {
		failed(idf, badopen);
	}
	return (rc);
}

void
rename(int f1, int f2)
{
	if (f1 != f2) {
		dup2(f1, f2);
		close(f1);
		if (f2 == 0) {
			ioset |= 1;
		}
	}
}

int
create(unsigned char *s)
{
	int rc;

	if ((rc = creat((const char *)s, 0666)) < 0) {
		failed(s, badcreate);
	}

	return (rc);
}

int
tmpfil(void)
{
	itos(serial++);
	movstr(numbuf, tmpnam);
	return (create(tmpout));
}

/* set by trim */
BOOL nosubst;

void
copy(struct ionod *ioparg)
{
	unsigned char c, *ends;
	unsigned char *cline, *clinep;
	int fd;
	struct ionod *iop;

	if ((iop = ioparg)) {
		copy(iop->iolst);
		ends = mactrim(iop->ioname);
		if (nosubst) {
			iop->iofile &= ~IODOC;
		}
		fd = tmpfil();
		iop->ioname = cpystak(tmpout);
		iop->iolst = iotemp;
		iotemp = iop;
		cline = locstak();

		for (;;) {
			clinep = cline;
			chkpr(NL);
			while ((c = (nosubst ? readc() : nextc(*ends)),
				!eolchar(c))) {
				*clinep++ = c;
			}
			*clinep = 0;
			if (eof || eq(cline, ends)) {
				break;
			}
			*clinep++ = NL;
			write(fd, cline, clinep - cline);
		}
		close(fd);
	}
}
