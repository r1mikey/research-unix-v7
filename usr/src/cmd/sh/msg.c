#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "sym.h"

const unsigned char version[] = "\nVERSION sys137	DATE 1978 Nov 6 14:29:22\n";

/* error messages */
const unsigned char badopt[] = "bad option(s)";
const unsigned char mailmsg[] = "you have mail\n";
const unsigned char nospace[] = "no space";
const unsigned char synmsg[] = "syntax error";

const unsigned char badnum[] = "bad number";
const unsigned char badparam[] = "parameter not set";
const unsigned char badsub[] = "bad substitution";
const unsigned char badcreate[] = "cannot create";
const unsigned char illegal[] = "illegal io";
const unsigned char restricted[] = "restricted";
const unsigned char piperr[] = "cannot make pipe";
const unsigned char badopen[] = "cannot open";
const unsigned char coredump[] = " - core dumped";
const unsigned char arglist[] = "arg list too long";
const unsigned char txtbsy[] = "text busy";
const unsigned char toobig[] = "too big";
const unsigned char badexec[] = "cannot execute";
const unsigned char notfound[] = "not found";
const unsigned char badfile[] = "bad file number";
const unsigned char badshift[] = "cannot shift";
const unsigned char baddir[] = "bad directory";
const unsigned char badtrap[] = "bad trap";
const unsigned char wtfailed[] = "is read only";
const unsigned char notid[] = "is not an identifier";

/* built in names */
const unsigned char pathname[] = "PATH";
const unsigned char homename[] = "HOME";
const unsigned char mailname[] = "MAIL";
const unsigned char fngname[] = "FILEMATCH";
const unsigned char ifsname[] = "IFS";
const unsigned char ps1name[] = "PS1";
const unsigned char ps2name[] = "PS2";

/* string constants */
const unsigned char nullstr[] = "";
const unsigned char sptbnl[] = " \t\n";
const unsigned char defpath[] = ":/bin:/usr/bin";
const unsigned char colon[] = ": ";
const unsigned char minus[] = "-";
const unsigned char endoffile[] = "end of file";
const unsigned char unexpected[] = " unexpected";
const unsigned char atline[] = " at line ";
const unsigned char devnull[] = "/dev/null";
const unsigned char execpmsg[] = "+ ";
const unsigned char readmsg[] = "> ";
const unsigned char stdprompt[] = "$ ";
const unsigned char supprompt[] = "# ";
const unsigned char profile[] = ".profile";

/* tables */
static const unsigned char sinsym[] = "in";
static const unsigned char sessym[] = "esac";
static const unsigned char scasym[] = "case";
static const unsigned char sfosym[] = "for";
static const unsigned char sodsym[] = "done";
static const unsigned char sifsym[] = "if";
static const unsigned char swhsym[] = "while";
static const unsigned char sdosym[] = "do";
static const unsigned char sthsym[] = "then";
static const unsigned char selsym[] = "else";
static const unsigned char sefsym[] = "elif";
static const unsigned char sfisym[] = "fi";
static const unsigned char sunsym[] = "until";
static const unsigned char sbrsym[] = "{";
static const unsigned char sktsym[] = "}";

const struct sysnod reserved[] = {
	{ sinsym, INSYM },
	{ sessym, ESSYM },
	{ scasym, CASYM },
	{ sfosym, FORSYM },
	{ sodsym, ODSYM },
	{ sifsym, IFSYM },
	{ swhsym, WHSYM },
	{ sdosym, DOSYM },
	{ sthsym, THSYM },
	{ selsym, ELSYM },
	{ sefsym, EFSYM },
	{ sfisym, FISYM },
	{ sunsym, UNSYM },
	{ sbrsym, BRSYM },
	{ sktsym, KTSYM },
	{ 0, 0 },
};

static const unsigned char smhup[] = "Hangup";
static const unsigned char smquit[] = "Quit";
static const unsigned char smill[] = "Illegal instruction";
static const unsigned char smtrc[] = "Trace/BPT trap";
static const unsigned char smiot[] = "IOT trap";
static const unsigned char smemt[] = "EMT trap";
static const unsigned char smfpe[] = "Floating exception";
static const unsigned char smkill[] = "Killed";
static const unsigned char smbus[] = "Bus error";
static const unsigned char smsegv[] = "Memory fault";
static const unsigned char smbadsys[] = "Bad system call";
static const unsigned char smalrm[] = "Alarm call";
static const unsigned char smterm[] = "Terminated";
static const unsigned char sms16[] = "Signal 16";

const unsigned char *sysmsg[] = {
	0,
	smhup,
	0, /* Interrupt */
	smquit,
	smill,
	smtrc,
	smiot,
	smemt,
	smfpe,
	smkill,
	smbus,
	smsegv,
	smbadsys,
	0, /* Broken pipe */
	smalrm,
	smterm,
	sms16,
};

const unsigned char export[] = "export";
const unsigned char readonly[] = "readonly";

static const unsigned char cdcmd[] = "cd";
static const unsigned char readcmd[] = "read";
#if 0
static const unsigned char tstbcmd[] = "[";
#endif
static const unsigned char setcmd[] = "set";
static const unsigned char coloncmd[] = ":";
static const unsigned char trapcmd[] = "trap";
static const unsigned char logincmd[] = "login";
static const unsigned char waitcmd[] = "wait";
static const unsigned char evalcmd[] = "eval";
static const unsigned char dotcmd[] = ".";
static const unsigned char nwgrpcmd[] = "newgrp";
static const unsigned char chdircmd[] = "chdir";
static const unsigned char brkcmd[] = "break";
static const unsigned char contcmd[] = "continue";
static const unsigned char shftcmd[] = "shift";
static const unsigned char exitcmd[] = "exit";
static const unsigned char execcmd[] = "exec";
static const unsigned char tmscmd[] = "times";
static const unsigned char umskcmd[] = "umask";

const struct sysnod commands[] = {
	{ cdcmd, SYSCD },
	{ readcmd, SYSREAD },
#if 0
	{tstbcmd, SYSTST},
#endif
	{ setcmd, SYSSET },
	{ coloncmd, SYSNULL },
	{ trapcmd, SYSTRAP },
	{ logincmd, SYSLOGIN },
	{ waitcmd, SYSWAIT },
	{ evalcmd, SYSEVAL },
	{ dotcmd, SYSDOT },
	{ nwgrpcmd, SYSLOGIN },
	{ readonly, SYSRDONLY },
	{ export, SYSXPORT },
	{ chdircmd, SYSCD },
	{ brkcmd, SYSBREAK },
	{ contcmd, SYSCONT },
	{ shftcmd, SYSSHFT },
	{ exitcmd, SYSEXIT },
	{ execcmd, SYSEXEC },
	{ tmscmd, SYSTIMES },
	{ umskcmd, SYSUMASK },
	{ 0, 0 },
};
