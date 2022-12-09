/* @(#)utsname.h	1.1 */

#ifndef __V7_SYS_UTSNAME_H
#define __V7_SYS_UTSNAME_H

struct utsname {
	char	sysname[9];
	char	nodename[9];
	char	release[9];
	char	version[9];
	char	machine[9];
};
extern struct utsname utsname;

#endif /* __V7_SYS_UTSNAME_H */
