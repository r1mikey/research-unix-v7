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
#include "sym.h"
#include "timeout.h"
#include <sys/types.h>

int output = 2;
static BOOL beenhere = FALSE;
unsigned char tmpout[20] = "/tmp/sh-";
FILEBLK stdfile;
struct fileblk *standin = &stdfile;

static void exfile(BOOL prof);

struct ionod *iopend;

int dolc;
unsigned char **dolv;

int wdval;
int wdnum;
struct argnod *wdarg;
int wdset;
BOOL reserv;

unsigned char *cmdadr;

unsigned char *pidadr;
unsigned char *tmpnam;

int serial;
struct fileblk *standin;

int peekc;
unsigned char *comdiv;

int flags;

jmp_buf subshell;
jmp_buf errshell;

BOOL trapnote;

int exitval;
BOOL execbrk;
int loopcnt;
int breakcnt;

int
main(int c, char *v[])
{
	int rflag = ttyflg;

	/* initialise storage allocation */
	stdsigs();
	setbrk(BRKINCR);
	addblok((unsigned int)0);

	/* set names from userenv */
	getenv();

	/* look for restricted */
	/*	if( c>0 && any('r', *v) ){ rflag=0 ;} */

	/* look for options */
	dolc = options(c, (unsigned char **)v);
	if (dolc < 2) {
		flags |= stdflg;
	}
	if ((flags & stdflg) == 0) {
		dolc--;
	}
	dolv = ((unsigned char **)v) + c - dolc;
	dolc--;

	/* return here for shell file execution */
	setjmp(subshell);

	/* number of positional parameters */
	assnum(&dolladr, dolc);
	cmdadr = dolv[0];

	/* set pidname */
	assnum(&pidadr, getpid());

	/* set up temp file names */
	settmp();

	/* default ifs */
	dfault(&ifsnod, (unsigned char *)sptbnl);

	if ((beenhere++) == FALSE) { /* ? profile */
		if (*cmdadr == '-' &&
		    (input = pathopen((unsigned char *)nullstr, (unsigned char *)profile)) >= 0) {
			exfile(rflag);
			flags &= ~ttyflg;
		}
		if (rflag == 0) {
			flags |= rshflg;
		}

		/* open input file if specified */
		if (comdiv) {
			estabf(comdiv);
			input = -1;
		} else {
			input = ((flags & stdflg) ? 0 : chkopen(cmdadr));
			comdiv--;
		}
	}

	exfile(0);
	done();
	return 0; /* UNREACH */
}

static void
exfile(BOOL prof)
{
	long int mailtime = 0;
	int userid;
	struct stat statb;
	struct sgttyb gttybuf;

	/* move input */
	if (input > 0) {
		Ldup(input, INIO);
		input = INIO;
	}

	/* move output to safe place */
	if (output == 2) {
		Ldup(dup(2), OTIO);
		output = OTIO;
	}

	userid = getuid();

	/* decide whether interactive */
	if ((flags & intflg) ||
	    ((flags & oneflg) == 0 && gtty(output, &gttybuf) == 0 &&
	     gtty(input, &gttybuf) == 0)) {
		dfault(&ps1nod, (unsigned char *)(userid ? stdprompt : supprompt));
		dfault(&ps2nod, (unsigned char *)readmsg);
		flags |= ttyflg | prompt;
		ignsig(KILL);
	} else {
		flags |= prof;
		flags &= ~prompt;
	}

	if (setjmp(errshell) && prof) {
		close(input);
		return;
	}

	/* error return here */
	loopcnt = breakcnt = peekc = 0;
	iopend = 0;
	if (input >= 0) {
		initf(input);
	}

	/* command loop */
	for (;;) {
		tdystak(0);
		stakchk(); /* may reduce sbrk */
		exitset();
		if ((flags & prompt) && standin->fstak == 0 && !eof) {
			if (mailnod.namval &&
			    stat((const char *)mailnod.namval, &statb) >= 0 &&
			    statb.st_size && (statb.st_mtime != mailtime) &&
			    mailtime) {
				prs(mailmsg);
			}
			mailtime = statb.st_mtime;
			prs(ps1nod.namval);
			alarm(TIMEOUT);
			flags |= waiting;
		}

		trapnote = 0;
		peekc = readc();
		if (eof) {
			return;
		}
		alarm(0);
		flags &= ~waiting;
		execute(cmd(NL, MTFLG), 0, NIL, NIL);
		eof |= (flags & oneflg);
	}
}

void
chkpr(char eor)
{
	if ((flags & prompt) && standin->fstak == 0 && eor == NL) {
		prs(ps2nod.namval);
	}
}

void
settmp(void)
{
	itos(getpid());
	serial = 0;
	tmpnam = movstr(numbuf, &tmpout[TMPNAM]);
}

void
Ldup(int fa, int fb)
{
	dup2(fa, fb);
	close(fa);
	ioctl(fb, FIOCLEX, 0);
}
