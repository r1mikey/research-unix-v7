/*
 *	execlp(name, arg,...,0)	(like execl, but does path search)
 *	execvp(name, argv)	(like execv, but does path search)
 */
#include <errno.h>
#define	NULL	0

static	char shell[] =	"/bin/sh";
char	*getenv();
extern	errno;

static char * execat(char *s1, char *s2, char *si);

static int __vexeclp(char *name, __builtin_va_list va)
{
	char *args[256];
	int idx;
	char *s;

	idx = 0;

	do {
		s = __builtin_va_arg(va, char *);
		args[idx++] = s;
	} while (s != (char *)0 && idx < 256);

	args[255] = (char *)0;

	return execvp(name, args);
}


int execlp(char *name, ...)
{
	int ret;
	__builtin_va_list va;
	__builtin_va_start(va, name);
	ret = __vexeclp(name, va);
	__builtin_va_end(va);
	return ret;
}

execvp(name, argv)
char *name, **argv;
{
	char *pathstr;
	register char *cp;
	char fname[128];
	char *newargs[256];
	int i;
	register unsigned etxtbsy = 1;
	register eacces = 0;

	if ((pathstr = getenv("PATH")) == NULL)
		pathstr = ":/bin:/usr/bin";
	cp = index(name, '/')? "": pathstr;

	do {
		cp = execat(cp, name, fname);
	retry:
		execv(fname, argv);
		switch(errno) {
		case ENOEXEC:
			newargs[0] = "sh";
			newargs[1] = fname;
			for (i=1; newargs[i+1]=argv[i]; i++) {
				if (i>=254) {
					errno = E2BIG;
					return(-1);
				}
			}
			execv(shell, newargs);
			return(-1);
		case ETXTBSY:
			if (++etxtbsy > 5)
				return(-1);
			sleep(etxtbsy);
			goto retry;
		case EACCES:
			eacces++;
			break;
		case ENOMEM:
		case E2BIG:
			return(-1);
		}
	} while (cp);
	if (eacces)
		errno = EACCES;
	return(-1);
}

static char *
execat(s1, s2, si)
register char *s1, *s2;
char *si;
{
	register char *s;

	s = si;
	while (*s1 && *s1 != ':' && *s1 != '-')
		*s++ = *s1++;
	if (si != s)
		*s++ = '/';
	while (*s2)
		*s++ = *s2++;
	*s = '\0';
	return(*s1? ++s1: 0);
}
