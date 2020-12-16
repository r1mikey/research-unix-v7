#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "sym.h"

static struct ionod * inout(struct ionod *lastio);
static void chkword();
static void chksym(int sym);
static struct trenod * term(int flg);
static struct trenod * makelist(int type, struct trenod *i, struct trenod *r);
static struct trenod * list(int flg);
static struct regnod * syncase(int esym);
static struct trenod * item(BOOL flag);
static int skipnl();
static void prsym(int sym);
static void synbad();

/* ========	command line decoding	========*/

struct trenod * makefork(int flgs, struct trenod *i)
{
	struct forknod *t;

	t = forkptr(getstak(FORKTYPE));
	t->forktyp = flgs | TFORK;
	t->forktre = i;
	t->forkio = 0;
	return treptr(t);
}

static struct trenod * makelist(int type, struct trenod *i, struct trenod *r)
{
	struct lstnod *t;

	if (i == 0 || r == 0) {
		synbad();
	} else {
		t = lstptr(getstak(LSTTYPE));
		t->lsttyp = type;
		t->lstlef = i;
		t->lstrit = r;
	}
	return treptr(t);
}

/*
 * cmd
 *	empty
 *	list
 *	list & [ cmd ]
 *	list [ ; cmd ]
 */

struct trenod * cmd(int sym, int flg)
{
	struct trenod *i, *e;

	i = list(flg);

	if (wdval == NL) {
		if (flg & NLFLG) {
			wdval = ';';
			chkpr(NL);
		}
	} else if (i == 0 && (flg & MTFLG) == 0) {
		synbad();
	}

	switch (wdval) {

	case '&':
		if (i) {
			i = makefork(FINT | FPRS | FAMP, i);
		} else {
			synbad();
		}

	case ';':
		if (e = cmd(sym, flg | MTFLG)) {
			i = makelist(TLST, i, e);
		}
		break;

	case EOFSYM:
		if (sym == NL) {
			break;
		}

	default:
		if (sym) {
			chksym(sym);
		}
	}
	return (i);
}

/*
 * list
 *	term
 *	list && term
 *	list || term
 */

static struct trenod * list(int flg)
{
	struct trenod *r;
	int b;

	r = term(flg);
	while (r && ((b = (wdval == ANDFSYM)) || wdval == ORFSYM)) {
		r = makelist((b ? TAND : TORF), r, term(NLFLG));
	}
	return (r);
}

/*
 * term
 *	item
 *	item |^ term
 */

static struct trenod * term(int flg)
{
	struct trenod *t;

	reserv++;
	if (flg & NLFLG) {
		skipnl();
	} else {
		word();
	}

	if ((t = item(TRUE)) && (wdval == '^' || wdval == '|')) {
		return (makelist(TFIL, makefork(FPOU, t),
				 makefork(FPIN | FPCL, term(NLFLG))));
	} else {
		return (t);
	}
}

static struct regnod * syncase(int esym)
{
	skipnl();
	if (wdval == esym) {
		return (0);
	} else {
		struct regnod *r = (struct regnod *)getstak(REGTYPE);
		r->regptr = 0;
		for (;;) {
			wdarg->argnxt = r->regptr;
			r->regptr = wdarg;
			if (wdval || (word() != ')' && wdval != '|')) {
				synbad();
			}
			if (wdval == '|') {
				word();
			} else {
				break;
			}
		}
		r->regcom = cmd(0, NLFLG | MTFLG);
		if (wdval == ECSYM) {
			r->regnxt = syncase(esym);
		} else {
			chksym(esym);
			r->regnxt = 0;
		}
		return (r);
	}
}

/*
 * item
 *
 *	( cmd ) [ < in  ] [ > out ]
 *	word word* [ < in ] [ > out ]
 *	if ... then ... else ... fi
 *	for ... while ... do ... done
 *	case ... in ... esac
 *	begin ... end
 */

static struct trenod * item(BOOL flag)
{
	struct trenod *t;
	struct ionod *io;

	if (flag) {
		io = inout(ioptr(0));
	} else {
		io = 0;
	}

	switch (wdval) {

	case CASYM: {
		t = treptr(getstak(SWTYPE));
		chkword();
		swptr(t)->swarg = wdarg->argval;
		skipnl();
		chksym(INSYM | BRSYM);
		swptr(t)->swlst = syncase(wdval == INSYM ? ESSYM : KTSYM);
		swptr(t)->swtyp = TSW;
		break;
	}

	case IFSYM: {
		int w;
		t = treptr(getstak(IFTYPE));
		ifptr(t)->iftyp = TIF;
		ifptr(t)->iftre = cmd(THSYM, NLFLG);
		ifptr(t)->thtre = cmd(ELSYM | FISYM | EFSYM, NLFLG);
		ifptr(t)->eltre =
		    ((w = wdval) == ELSYM
			 ? cmd(FISYM, NLFLG)
			 : (w == EFSYM ? (wdval = IFSYM, item(0)) : 0));
		if (w == EFSYM) {
			return (t);
		}
		break;
	}

	case FORSYM: {
		t = treptr(getstak(FORTYPE));
		forptr(t)->fortyp = TFOR;
		forptr(t)->forlst = 0;
		chkword();
		forptr(t)->fornam = wdarg->argval;
		if (skipnl() == INSYM) {
			chkword();
			forptr(t)->forlst = comptr(item(0));
			if (wdval != NL && wdval != ';') {
				synbad();
			}
			chkpr(wdval);
			skipnl();
		}
		chksym(DOSYM | BRSYM);
		forptr(t)->fortre =
		    cmd(wdval == DOSYM ? ODSYM : KTSYM, NLFLG);
		break;
	}

	case WHSYM:
	case UNSYM: {
		t = treptr(getstak(WHTYPE));
		whptr(t)->whtyp = (wdval == WHSYM ? TWH : TUN);
		whptr(t)->whtre = cmd(DOSYM, NLFLG);
		whptr(t)->dotre = cmd(ODSYM, NLFLG);
		break;
	}

	case BRSYM:
		t = cmd(KTSYM, NLFLG);
		break;

	case '(': {
		struct parnod *p;
		p = parptr(getstak(PARTYPE));
		p->partre = cmd(')', NLFLG);
		p->partyp = TPAR;
		t = makefork(0, treptr(p));
		break;
	}

	default:
		if (io == 0) {
			return (0);
		}

	case 0: {
		struct argnod *argp;
		struct argnod **argtail;
		struct argnod **argset = 0;
		int keywd = 1;
		t = treptr(getstak(COMTYPE));
		comptr(t)->comio = io; /*initial io chain*/
		argtail = &(comptr(t)->comarg);
		while (wdval == 0) {
			argp = wdarg;
			if (wdset && keywd) {
				argp->argnxt = argset;
				argset = argp;
			} else {
				*argtail = argp;
				argtail = &(argp->argnxt);
				keywd = flags & keyflg;
			}
			word();
			if (flag) {
				comptr(t)->comio = inout(comptr(t)->comio);
			};
		}

		comptr(t)->comtyp = TCOM;
		comptr(t)->comset = argset;
		*argtail = 0;
		return (t);
	}
	}
	reserv++;
	word();
	if (io = inout(io)) {
		t = makefork(0, t);
		t->treio = io;
	}
	return (t);
}

static int
skipnl()
{
	while ((reserv++, word() == NL)) {
		chkpr(NL);
	}
	return (wdval);
}

static struct ionod * inout(struct ionod *lastio)
{
	int iof;
	struct ionod *iop;
	char c;

	iof = wdnum;

	switch (wdval) {

	case DOCSYM:
		iof |= IODOC;
		break;

	case APPSYM:
	case '>':
		if (wdnum == 0) {
			iof |= 1;
		}
		iof |= IOPUT;
		if (wdval == APPSYM) {
			iof |= IOAPP;
			break;
		}

	case '<':
		if ((c = nextc(0)) == '&') {
			iof |= IOMOV;
		} else if (c == '>') {
			iof |= IORDW;
		} else {
			peekc = c | MARK;
		}
		break;

	default:
		return (lastio);
	}

	chkword();
	iop = (struct ionod *)getstak(IOTYPE);
	iop->ioname = wdarg->argval;
	iop->iofile = iof;
	if (iof & IODOC) {
		iop->iolst = iopend;
		iopend = iop;
	}
	word();
	iop->ionxt = inout(lastio);
	return (iop);
}

static void
chkword()
{
	if (word()) {
		synbad();
	}
}

static void chksym(int sym)
{
	int x = sym & wdval;
	if (((x & SYMFLG) ? x : sym) != wdval) {
		synbad();
	}
}

static void prsym(int sym)
{
	if (sym & SYMFLG) {
		struct sysnod *sp = reserved;
		while (sp->sysval && sp->sysval != sym) {
			sp++;
		}
		prs(sp->sysnam);
	} else if (sym == EOFSYM) {
		prs(endoffile);
	} else {
		if (sym & SYMREP) {
			prc(sym);
		}
		if (sym == NL) {
			prs("newline");
		} else {
			prc(sym);
		};
	}
}

static void
synbad()
{
	prp();
	prs(synmsg);
	if ((flags & ttyflg) == 0) {
		prs(atline);
		prn(standin->flin);
	}
	prs(colon);
	prc(LQ);
	if (wdval) {
		prsym(wdval);
	} else {
		prs(wdarg->argval);
	}
	prc(RQ);
	prs(unexpected);
	newline();
	exitsh(SYNBAD);
}
