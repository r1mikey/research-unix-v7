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

#include <sgtty.h>
#include <sys/stat.h>
#include <sys/times.h>

/* system calls called by the shell */
extern int stat(const char *path, struct stat *buf);
extern int unlink(const char *path);
extern int pipe(int fildes[2]);
extern int creat(const char *path, int mode);
extern int open(const char *path, int oflag);
extern int dup(int fildes);
extern int dup2(int fildes, int fildes2);
extern int lseek(int fildes, int offset, int whence);
extern int read(int fildes, void *buf, unsigned int nbyte);
extern int write(int fildes, const void *buf, unsigned int nbyte);
extern int close(int fildes);

extern int ioctl(int fdes, int cmd, void *arg);
extern int gtty(int fd, struct sgttyb *ap);

extern int fork(void);
extern int execve(const char *path, char *const argv[], char *const envp[]);
extern void exit(int status);
extern int wait(int *sts);
extern int times(struct tms *tp);

extern int getpid(void);
extern int getuid(void);
extern int umask(int mask);
extern int chdir(const char *fname);

extern int alarm(int deltat);
extern int pause(void);

typedef void (*sig_t)(int);
extern sig_t signal(int sig, sig_t func);

extern void *sbrk(int incr);

#include "mac.h"
#include "mode.h"
#include "name.h"

/* result type declarations */
#define alloc malloc

/* args.c */
extern int options(int argc, unsigned char **argv);
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
extern int builtin(int argn, unsigned char **com);

/* cmd.c */
extern struct trenod * makefork(int flgs, struct trenod *i);
extern struct trenod * cmd(int sym, int flg);

/* error.c */
extern void exitset(void);
extern void sigchk();
extern void failed(const unsigned char *s1, const unsigned char *s2);
extern void error(const unsigned char *s);
extern void exitsh(int xno);
extern void done(void);
extern void rmtemp(struct ionod *base);

/* expand.c */
extern int expand(unsigned char *as, int rflg);
extern int gmatch(const unsigned char *s, unsigned char *p);
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
extern int estabf(unsigned char *s);
extern void push(struct fileblk *af);
extern BOOL pop(void);
extern void chkpipe(int *pv);
extern int chkopen(const unsigned char *idf);
extern void rename(int f1, int f2);
extern int create(unsigned char *s);
extern int tmpfil(void);
extern void copy(struct ionod *ioparg);

/* macro.c */
extern unsigned char * macro(unsigned char *as);
extern void subst(int in, int ot);

/* main.c */
extern void chkpr(char eor);
extern void settmp(void);
extern void Ldup(int fa, int fb);

/* name.c */
extern int syslook(const unsigned char *w, const struct sysnod syswds[]);
extern void setlist(struct argnod *arg, int xp);
extern void setname(unsigned char *argi, int xp);
extern void replace(unsigned char **a, unsigned char *v);
extern void dfault(struct namnod *n, unsigned char *v);
extern void assign(struct namnod *n, unsigned char *v);
extern int readvar(unsigned char **names);
extern void assnum(unsigned char **p, int i);
extern unsigned char * make(const unsigned char *v);
extern struct namnod * lookup(const unsigned char *nam);
extern void namscan(void (*fn)(struct namnod *));
extern void printnam(struct namnod *n);
extern void exname(struct namnod *n);
extern void printflg(struct namnod *n);
extern void getenv(void);
extern void countnam(struct namnod *n);
extern void pushnam(struct namnod *n);
extern unsigned char ** setenv(void);

/* print.c */
extern void newline(void);
extern void blank(void);
extern void prp(void);
extern void prs(const unsigned char *as);
extern void prc(unsigned char c);
extern void prt(long int t);
extern void prn(int n);
extern void itos(int n);
extern int stoi(const unsigned char *icp);

/* service.c */
extern void initio(struct ionod *iop);
extern unsigned char * getpath(unsigned char *s);
extern int pathopen(unsigned char *path, unsigned char *name);
extern unsigned char * catpath(unsigned char *path, unsigned char *name);
extern void execa(unsigned char *at[]);
extern void postclr(void);
extern void post(int pcsid);
extern void await(int i);
extern void trim(unsigned char *at);
extern unsigned char * mactrim(unsigned char *s);
extern unsigned char ** scan(int argn);
extern int getarg(struct comnod *ac);

/* setbrk.c */
extern unsigned char * setbrk(int incr);

/* string.c */
extern unsigned char * movstr(const unsigned char *a, unsigned char *b);
extern int any(unsigned char c, const unsigned char *s);
extern int cf(const unsigned char *s1, const unsigned char *s2);
extern int length(const unsigned char *as);

/* word.c */
extern int word(void);
extern unsigned char nextc(unsigned char quote);
extern unsigned char readc(void);

/* xec.c */
extern int execute(struct trenod *argt, int execflg, int *pf1, int *pf2);
extern void execexp(unsigned char *s, int f);

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
#define STK(x) ((unsigned char *)(x))
#define ADR(x) ((unsigned char *)(x))

/* stak stuff */
#include "stak.h"

/* string constants */
extern const unsigned char atline[];
extern const unsigned char readmsg[];
extern const unsigned char colon[];
extern const unsigned char minus[];
extern const unsigned char nullstr[];
extern const unsigned char sptbnl[];
extern const unsigned char unexpected[];
extern const unsigned char endoffile[];
extern const unsigned char synmsg[];

/* name tree and words */
extern const struct sysnod reserved[];
extern int wdval;
extern int wdnum;
extern struct argnod *wdarg;
extern int wdset;
extern BOOL reserv;

/* prompting */
extern const unsigned char stdprompt[];
extern const unsigned char supprompt[];
extern const unsigned char profile[];

/* built in names */
extern struct namnod fngnod;
extern struct namnod ifsnod;
extern struct namnod homenod;
extern struct namnod mailnod;
extern struct namnod pathnod;
extern struct namnod ps1nod;
extern struct namnod ps2nod;

/* special names */
unsigned extern char flagadr[];
extern unsigned char *cmdadr;
extern unsigned char *exitadr;
extern unsigned char *dolladr;
extern unsigned char *pcsadr;
extern unsigned char *pidadr;

extern const unsigned char defpath[];

/* names always present */
extern const unsigned char mailname[];
extern const unsigned char homename[];
extern const unsigned char pathname[];
extern const unsigned char fngname[];
extern const unsigned char ifsname[];
extern const unsigned char ps1name[];
extern const unsigned char ps2name[];

/* transput */
extern unsigned char tmpout[];
extern unsigned char *tmpnam;
extern int serial;
#define TMPNAM 7
extern struct fileblk *standin;
#define input (standin->fdes)
#define eof   (standin->feof)
extern int peekc;
extern unsigned char *comdiv;
extern const unsigned char devnull[];

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
extern unsigned char *trapcom[];
extern BOOL trapflg[];

/* name tree and words */
extern char **environ;
extern unsigned char numbuf[];
extern const unsigned char export[];
extern const unsigned char readonly[];

/* execflgs */
extern int exitval;
extern BOOL execbrk;
extern int loopcnt;
extern int breakcnt;

/* messages */
extern const unsigned char mailmsg[];
extern const unsigned char coredump[];
extern const unsigned char badopt[];
extern const unsigned char badparam[];
extern const unsigned char badsub[];
extern const unsigned char nospace[];
extern const unsigned char notfound[];
extern const unsigned char badtrap[];
extern const unsigned char baddir[];
extern const unsigned char badshift[];
extern const unsigned char illegal[];
extern const unsigned char restricted[];
extern const unsigned char execpmsg[];
extern const unsigned char notid[];
extern const unsigned char wtfailed[];
extern const unsigned char badcreate[];
extern const unsigned char piperr[];
extern const unsigned char badopen[];
extern const unsigned char badnum[];
extern const unsigned char arglist[];
extern const unsigned char txtbsy[];
extern const unsigned char toobig[];
extern const unsigned char badexec[];
extern const unsigned char badfile[];

extern address _end[];

#include "ctype.h"
