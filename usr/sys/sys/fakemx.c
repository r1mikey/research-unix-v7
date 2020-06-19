/*
 * Fake multiplexor routines to satisfy references
 * if you don't want it.
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/inode.h"
#include "../h/mx.h"

void sdata(struct chan *cp)
{
}

void mcttstart(struct tty *tp)
{
}

void mpxchan(void)
{
	u.u_error = EINVAL;
}

void mcstart(struct chan *p, caddr_t q)
{
}

void scontrol(struct chan *chan, int s, int c)
{
}
