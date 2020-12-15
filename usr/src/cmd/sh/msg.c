#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */


#include	"defs.h"
#include	"sym.h"

char		version[] = "\nVERSION sys137	DATE 1978 Nov 6 14:29:22\n";

/* error messages */
char	badopt[]		= "bad option(s)";
char	mailmsg[]		= "you have mail\n";
char	nospace[]		= "no space";
char	synmsg[]		= "syntax error";

char	badnum[]		= "bad number";
char	badparam[]	= "parameter not set";
char	badsub[]		= "bad substitution";
char	badcreate[]	= "cannot create";
char	illegal[]		= "illegal io";
char	restricted[]	= "restricted";
char	piperr[]		= "cannot make pipe";
char	badopen[]		= "cannot open";
char	coredump[]	= " - core dumped";
char	arglist[]		= "arg list too long";
char	txtbsy[]		= "text busy";
char	toobig[]		= "too big";
char	badexec[]		= "cannot execute";
char	notfound[]	= "not found";
char	badfile[]		= "bad file number";
char	badshift[]	= "cannot shift";
char	baddir[]		= "bad directory";
char	badtrap[]		= "bad trap";
char	wtfailed[]	= "is read only";
char	notid[]		= "is not an identifier";

/* built in names */
char	pathname[]	= "PATH";
char	homename[]	= "HOME";
char	mailname[]	= "MAIL";
char	fngname[]		= "FILEMATCH";
char	ifsname[]		= "IFS";
char	ps1name[]		= "PS1";
char	ps2name[]		= "PS2";

/* string constants */
char	nullstr[]		= "";
char	sptbnl[]		= " \t\n";
char	defpath[]		= ":/bin:/usr/bin";
char	colon[]		= ": ";
char	minus[]		= "-";
char	endoffile[]	= "end of file";
char	unexpected[] 	= " unexpected";
char	atline[]		= " at line ";
char	devnull[]		= "/dev/null";
char	execpmsg[]	= "+ ";
char	readmsg[]		= "> ";
char	stdprompt[]	= "$ ";
char	supprompt[]	= "# ";
char	profile[]		= ".profile";


/* tables */
struct sysnod reserved[] = {
		{"in",		INSYM},
		{"esac",	ESSYM},
		{"case",	CASYM},
		{"for",		FORSYM},
		{"done",	ODSYM},
		{"if",		IFSYM},
		{"while",	WHSYM},
		{"do",		DOSYM},
		{"then",	THSYM},
		{"else",	ELSYM},
		{"elif",	EFSYM},
		{"fi",		FISYM},
		{"until",	UNSYM},
		{ "{",		BRSYM},
		{ "}",		KTSYM},
		{0,	0},
};

char *	sysmsg[] = {
		0,
		"Hangup",
		0,	/* Interrupt */
		"Quit",
		"Illegal instruction",
		"Trace/BPT trap",
		"IOT trap",
		"EMT trap",
		"Floating exception",
		"Killed",
		"Bus error",
		"Memory fault",
		"Bad system call",
		0,	/* Broken pipe */
		"Alarm call",
		"Terminated",
		"Signal 16",
};

char		export[] = "export";
char		readonly[] = "readonly";
struct sysnod 	commands[] = {
		{"cd",		SYSCD},
		{"read",	SYSREAD},
/*
		{"[",		SYSTST},
*/
		{"set",		SYSSET},
		{":",		SYSNULL},
		{"trap",	SYSTRAP},
		{"login",	SYSLOGIN},
		{"wait",	SYSWAIT},
		{"eval",	SYSEVAL},
		{".",		SYSDOT},
		{"newgrp",	SYSLOGIN},
		{readonly,	SYSRDONLY},
		{export,	SYSXPORT},
		{"chdir",	SYSCD},
		{"break",	SYSBREAK},
		{"continue",	SYSCONT},
		{"shift",	SYSSHFT},
		{"exit",	SYSEXIT},
		{"exec",	SYSEXEC},
		{"times",	SYSTIMES},
		{"umask",	SYSUMASK},
		{0,	0},
};
