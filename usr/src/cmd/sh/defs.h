#
/*
 *	UNIX shell
 */

/* error exits from various parts of shell */
#define ERROR	1
#define SYNBAD	2
#define SIGFAIL 3
#define SIGFLG	0200

/* command tree */
#define FPRS	020
#define FINT	040
#define FAMP	0100
#define FPIN	0400
#define FPOU	01000
#define FPCL	02000
#define FCMD	04000
#define COMMSK	017

#define TCOM	0
#define TPAR	1
#define TFIL	2
#define TLST	3
#define TIF	4
#define TWH	5
#define TUN	6
#define TSW	7
#define TAND	8
#define TORF	9
#define TFORK	10
#define TFOR	11

/* execute table */
#define SYSSET	1
#define SYSCD	2
#define SYSEXEC	3
#define SYSLOGIN 4
#define SYSTRAP	5
#define SYSEXIT	6
#define SYSSHFT 7
#define SYSWAIT	8
#define SYSCONT 9
#define SYSBREAK 10
#define SYSEVAL 11
#define SYSDOT	12
#define SYSRDONLY 13
#define SYSTIMES 14
#define SYSXPORT 15
#define SYSNULL 16
#define SYSREAD 17
#define SYSTST	18
#define	SYSUMASK	19

/* used for input and output of shell */
#define INIO 10
#define OTIO 11

/*io nodes*/
#define USERIO	10
#define IOUFD	15
#define IODOC	16
#define IOPUT	32
#define IOAPP	64
#define IOMOV	128
#define IORDW	256
#define INPIPE	0
#define OTPIPE	1

/* arg list terminator */
#define ENDARGS	0

#include	"mac.h"
#include	"mode.h"
#include	"name.h"


/* result type declarations */
#define alloc malloc
ADDRESS		alloc();
void		addblok();
STRING		make();
STRING		movstr();
TREPTR		cmd();
TREPTR		makefork();
NAMPTR		lookup();
void		setname();
void		setargs();
DOLPTR		useargs();
REAL		expr();
STRING		catpath();
STRING		getpath();
STRING		*scan();
STRING		mactrim();
STRING		macro();
void		await();
void		post();
void		exname();
void		printnam();
void		printflg();
void		prs();
void		prc();
void		getenv();
STRING		*setenv();

#define attrib(n,f)	(n->namflg |= f)
#define round(a,b)	(((int)((ADR(a)+b)-1))&~((b)-1))
#define closepipe(x)	(close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a,b)		(cf(a,b)==0)
#define max(a,b)	((a)>(b)?(a):(b))
#define assert(x)	;

/* temp files and io */
UFD		output;
INT		ioset;
IOPTR		iotemp;		/* files to be deleted sometime */
IOPTR		iopend;		/* documents waiting to be read at NL */

/* substitution */
INT		dolc;
STRING		*dolv;
DOLPTR		argfor;
ARGPTR		gchain;

/* stack */
#define		BLK(x)	((BLKPTR)(x))
#define		BYT(x)	((BYTPTR)(x))
#define		STK(x)	((STKPTR)(x))
#define		ADR(x)	((char*)(x))

/* stak stuff */
#include	"stak.h"

/* string constants */
extern MSG		atline;
extern MSG		readmsg;
extern MSG		colon;
extern MSG		minus;
extern MSG		nullstr;
extern MSG		sptbnl;
extern MSG		unexpected;
extern MSG		endoffile;
extern MSG		synmsg;

/* name tree and words */
extern struct sysnod		reserved[];
INT		wdval;
INT		wdnum;
ARGPTR		wdarg;
INT		wdset;
BOOL		reserv;

/* prompting */
extern MSG		stdprompt;
extern MSG		supprompt;
extern MSG		profile;

/* built in names */
NAMNOD		fngnod;
NAMNOD		ifsnod;
NAMNOD		homenod;
NAMNOD		mailnod;
NAMNOD		pathnod;
NAMNOD		ps1nod;
NAMNOD		ps2nod;

/* special names */
extern MSG		flagadr;
STRING		cmdadr;
STRING		exitadr;
STRING		dolladr;
STRING		pcsadr;
STRING		pidadr;

extern MSG		defpath;

/* names always present */
extern MSG		mailname;
extern MSG		homename;
extern MSG		pathname;
extern MSG		fngname;
extern MSG		ifsname;
extern MSG		ps1name;
extern MSG		ps2name;

/* transput */
extern CHAR		tmpout[];
STRING		tmpnam;
INT		serial;
#define		TMPNAM 7
FILE		standin;
#define input	(standin->fdes)
#define eof	(standin->feof)
INT		peekc;
STRING		comdiv;
extern MSG		devnull;

/* flags */
#define		noexec	01
#define		intflg	02
#define		prompt	04
#define		setflg	010
#define		errflg	020
#define		ttyflg	040
#define		forked	0100
#define		oneflg	0200
#define		rshflg	0400
#define		waiting	01000
#define		stdflg	02000
#define		execpr	04000
#define		readpr	010000
#define		keyflg	020000
INT		flags;

/* error exits from various parts of shell */
#include	<setjmp.h>
jmp_buf		subshell;
jmp_buf		errshell;

/* fault handling */
#include	"brkincr.h"
POS		brkincr;

#define MINTRAP	0
#define MAXTRAP	17

#define INTR	2
#define QUIT	3
#define MEMF	11
#define ALARM	14
#define KILL	15
#define TRAPSET	2
#define SIGSET	4
#define SIGMOD	8

void		fault();
BOOL		trapnote;
extern STRING		trapcom[];
extern BOOL		trapflg[];

/* name tree and words */
STRING		*environ;
extern CHAR		numbuf[];
extern MSG		export;
extern MSG		readonly;

/* execflgs */
INT		exitval;
BOOL		execbrk;
INT		loopcnt;
INT		breakcnt;

/* messages */
extern MSG		mailmsg;
extern MSG		coredump;
extern MSG		badopt;
extern MSG		badparam;
extern MSG		badsub;
extern MSG		nospace;
extern MSG		notfound;
extern MSG		badtrap;
extern MSG		baddir;
extern MSG		badshift;
extern MSG		illegal;
extern MSG		restricted;
extern MSG		execpmsg;
extern MSG		notid;
extern MSG		wtfailed;
extern MSG		badcreate;
extern MSG		piperr;
extern MSG		badopen;
extern MSG		badnum;
extern MSG		arglist;
extern MSG		txtbsy;
extern MSG		toobig;
extern MSG		badexec;
extern MSG		notfound;
extern MSG		badfile;

extern address	_end[];

#include	"ctype.h"

