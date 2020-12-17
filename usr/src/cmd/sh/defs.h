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

/* MICHAEL */

/* args.c */
extern struct dolnod * freeargs(struct dolnod *blk);
extern void setargs(unsigned char *argi[]);
extern void clearup(void);
extern struct dolnod * useargs(void);

/* blok.c */
extern void * alloc(unsigned int nbytes);
extern void addblok(unsigned int reqd);
extern void free(void *ap);
#ifdef DEBUG
extern void chkbptr(struct blk *ptr);
#endif

/* builtin.c */
extern int builtin(int argn, char **com);

/* cmd.c */
extern struct trenod * makefork(int flgs, struct trenod *i);
extern struct trenod * cmd(int sym, int flg);

/* error.c */
extern void exitset(void);
extern void sigchk();
extern void failed(char *s1, char *s2);
extern void error(char *s);
extern void exitsh(int xno);
extern void done(void);
extern void rmtemp(struct ionod *base);

/* expand.c */
extern int expand(char *as, int rflg);
extern int gmatch(char *s, char *p);
extern void makearg(struct argnod *args);

/* fault.c */
extern void stdsigs(void);
extern int ignsig(int n);
extern void getsig(int n);
extern void oldsigs(void);
extern void clrsig(int i);
extern void chktrap(void);

/* io.c */
extern void initf(int fd);
extern int estabf(char *s);
extern void push(struct fileblk *af);
extern BOOL pop(void);
extern void chkpipe(int *pv);
extern int chkopen(char *idf);
extern void rename(int f1, int f2);
extern int create(char *s);
extern int tmpfil(void);
extern void copy(struct ionod *ioparg);

/* macro.c */
extern char * macro(char *as);
extern void subst(int in, int ot);

/* main.c */
extern void chkpr(char eor);
extern void settmp(void);
extern void Ldup(int fa, int fb);

/* name.c */
extern int syslook(char *w, struct sysnod syswds[]);
extern void setlist(struct argnod *arg, int xp);
extern void setname(char *argi, int xp);
extern void replace(char **a, char *v);
extern void dfault(struct namnod *n, char *v);
extern void assign(struct namnod *n, char *v);
extern int readvar(char **names);
extern void assnum(char **p, int i);
extern char * make(char *v);
extern struct namnod * lookup(char *nam);
extern void namscan(void (*fn)(struct namnod *));
extern void printnam(struct namnod *n);
extern void exname(struct namnod *n);
extern void printflg(struct namnod *n);
extern void getenv(void);
extern void countnam(struct namnod *n);
extern void pushnam(struct namnod *n);
extern char ** setenv(void);

/* print.c */
extern void newline(void);
extern void blank(void);
extern void prp(void);
extern void prs(char *as);
extern void prc(char c);
extern void prt(long int t);
extern void prn(int n);
extern void itos(int n);
extern int stoi(char *icp);

/* service.c */
extern void initio(struct ionod *iop);
extern char * getpath(char *s);
extern int pathopen(char *path, char *name);
extern char * catpath(char *path, char *name);
extern void execa(char *at[]);
extern void postclr(void);
extern void post(int pcsid);
extern void await(int i);
extern void trim(char *at);
extern char * mactrim(char *s);
extern char ** scan(int argn);
extern int getarg(struct comnod *ac);

/* setbrk.c */
extern unsigned char * setbrk(int incr);

/* string.c */
extern char * movstr(char *a, char *b);
extern int any(char c, char *s);
extern int cf(char *s1, char *s2);
extern int length(char *as);

/* word.c */
extern int word(void);
extern char nextc(char quote);
extern char readc(void);

/* xec.c */
extern int execute(struct trenod *argt, int execflg, int *pf1, int *pf2);
extern void execexp(char *s, int f);

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
#define STK(x) ((char *)(x))
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
