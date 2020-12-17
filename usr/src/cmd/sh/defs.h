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
#define FPRS   020
#define FINT   040
#define FAMP   0100
#define FPIN   0400
#define FPOU   01000
#define FPCL   02000
#define FCMD   04000
#define COMMSK 017

#define TCOM  0
#define TPAR  1
#define TFIL  2
#define TLST  3
#define TIF   4
#define TWH   5
#define TUN   6
#define TSW   7
#define TAND  8
#define TORF  9
#define TFORK 10
#define TFOR  11

/* execute table */
#define SYSSET	  1
#define SYSCD	  2
#define SYSEXEC	  3
#define SYSLOGIN  4
#define SYSTRAP	  5
#define SYSEXIT	  6
#define SYSSHFT	  7
#define SYSWAIT	  8
#define SYSCONT	  9
#define SYSBREAK  10
#define SYSEVAL	  11
#define SYSDOT	  12
#define SYSRDONLY 13
#define SYSTIMES  14
#define SYSXPORT  15
#define SYSNULL	  16
#define SYSREAD	  17
#define SYSTST	  18
#define SYSUMASK  19

/* used for input and output of shell */
#define INIO 10
#define OTIO 11

/*io nodes*/
#define USERIO 10
#define IOUFD  15
#define IODOC  16
#define IOPUT  32
#define IOAPP  64
#define IOMOV  128
#define IORDW  256
#define INPIPE 0
#define OTPIPE 1

/* arg list terminator */
#define ENDARGS 0

#include "mac.h"
#include "mode.h"
#include "name.h"

/* result type declarations */
#define alloc malloc
void *alloc(unsigned int nbytes);
void addblok(unsigned int reqd);
char *make();
char *movstr();
struct trenod * cmd(int sym, int flg);
struct trenod * makefork(int flgs, struct trenod *i);
struct namnod * lookup();
void setname();
void setargs(unsigned char *argi[]);
struct dolnod * useargs();
char *catpath();
char *getpath();
char **scan();
void trim(char *at);
char *mactrim();
char *macro();
void await();
void post();
void exname();
void printnam();
void printflg();
void prs(char *as);
void prc(char c);
void prn(int n);
void itos(int n);
void getenv();
char **setenv();

typedef __INTPTR_TYPE__ intptr_t;
typedef __SIZE_TYPE__ size_t;

#define attrib(n, f) (n->namflg |= f)
#define	round(a, b)	(((intptr_t)(((char *)(a)+b)-1))&~((b)-1))
#define closepipe(x) (close(x[INPIPE]), close(x[OTPIPE]))
#define eq(a, b)     (cf(a, b) == 0)
#define max(a, b)    ((a) > (b) ? (a) : (b))
#define assert(x)    ;

/* temp files and io */
extern int output;
extern int ioset;
extern struct ionod *iotemp; /* files to be deleted sometime */
extern struct ionod *iopend; /* documents waiting to be read at NL */

/* substitution */
extern int dolc;
extern unsigned char **dolv;
extern struct dolnod *argfor;
extern struct argnod *gchain;

/* stack */
#define BLK(x) ((struct blk *)(x))
#define BYT(x) ((BYTPTR)(x))
#define STK(x) ((STKPTR)(x))
#define ADR(x) ((char *)(x))

/* stak stuff */
#include "stak.h"

/* string constants */
extern char atline[];
extern char readmsg[];
extern char colon[];
extern char minus[];
extern char nullstr[];
extern char sptbnl[];
extern char unexpected[];
extern char endoffile[];
extern char synmsg[];

/* name tree and words */
extern struct sysnod reserved[];
extern int wdval;
extern int wdnum;
extern struct argnod *wdarg;
extern int wdset;
extern BOOL reserv;

/* prompting */
extern char stdprompt[];
extern char supprompt[];
extern char profile[];

/* built in names */
extern NAMNOD fngnod;
extern NAMNOD ifsnod;
extern NAMNOD homenod;
extern NAMNOD mailnod;
extern NAMNOD pathnod;
extern NAMNOD ps1nod;
extern NAMNOD ps2nod;

/* special names */
extern char flagadr[];
extern char *cmdadr;
extern char *exitadr;
extern char *dolladr;
extern char *pcsadr;
extern char *pidadr;

extern char defpath[];

/* names always present */
extern char mailname[];
extern char homename[];
extern char pathname[];
extern char fngname[];
extern char ifsname[];
extern char ps1name[];
extern char ps2name[];

/* transput */
extern char tmpout[];
extern char *tmpnam;
extern int serial;
#define TMPNAM 7
extern struct fileblk *standin;
#define input (standin->fdes)
#define eof   (standin->feof)
extern int peekc;
extern char *comdiv;
extern char devnull[];

/* flags */
#define noexec	01
#define intflg	02
#define prompt	04
#define setflg	010
#define errflg	020
#define ttyflg	040
#define forked	0100
#define oneflg	0200
#define rshflg	0400
#define waiting 01000
#define stdflg	02000
#define execpr	04000
#define readpr	010000
#define keyflg	020000
extern int flags;

/* error exits from various parts of shell */
#include <setjmp.h>
extern jmp_buf subshell;
extern jmp_buf errshell;

/* fault handling */
#include "brkincr.h"
extern unsigned int brkincr;

#define MINTRAP 0
#define MAXTRAP 17

#define INTR	2
#define QUIT	3
#define MEMF	11
#define ALARM	14
#define KILL	15
#define TRAPSET 2
#define SIGSET	4
#define SIGMOD	8

void fault();
extern BOOL trapnote;
extern char *trapcom[];
extern BOOL trapflg[];

/* name tree and words */
extern char **environ;
extern char numbuf[];
extern char export[];
extern char readonly[];

/* execflgs */
extern int exitval;
extern BOOL execbrk;
extern int loopcnt;
extern int breakcnt;

/* messages */
extern char mailmsg[];
extern char coredump[];
extern char badopt[];
extern char badparam[];
extern char badsub[];
extern char nospace[];
extern char notfound[];
extern char badtrap[];
extern char baddir[];
extern char badshift[];
extern char illegal[];
extern char restricted[];
extern char execpmsg[];
extern char notid[];
extern char wtfailed[];
extern char badcreate[];
extern char piperr[];
extern char badopen[];
extern char badnum[];
extern char arglist[];
extern char txtbsy[];
extern char toobig[];
extern char badexec[];
extern char notfound[];
extern char badfile[];

extern address _end[];

#include "ctype.h"
