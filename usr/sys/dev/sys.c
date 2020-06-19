/*
 *	indirect driver for controlling tty.
 */
#include "../h/param.h"
#include "../h/conf.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/proc.h"

void syopen(dev_t dev, int flag)
{
	if(u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return;
	}
	(*cdevsw[major(u.u_ttyd)].d_open)(u.u_ttyd, flag);
}

void syread(dev_t dev)
{
	(*cdevsw[major(u.u_ttyd)].d_read)(u.u_ttyd);
}

void sywrite(dev_t dev)
{
	(*cdevsw[major(u.u_ttyd)].d_write)(u.u_ttyd);
}

void sysioctl(dev_t dev, int cmd, caddr_t addr, int flag)
{
	(*cdevsw[major(u.u_ttyd)].d_ioctl)(u.u_ttyd, cmd, addr, flag);
}
