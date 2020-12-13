#include "../../h/param.h"
#include "../../h/systm.h"
#include "../../h/buf.h"
#include "../../h/tty.h"
#include "../../h/conf.h"
#include "../../h/proc.h"
#include "../../h/text.h"
#include "../../h/dir.h"
#include "../../h/user.h"
#include "../../h/file.h"
#include "../../h/inode.h"
#include "../../h/acct.h"

int	nulldev();
int	nodev();
int     sdxstrategy();
extern struct	buf	sdxtab;

struct	bdevsw	bdevsw[] =
{
	{nodev, nodev, nodev, 0},	/* rk = 0 */
	{nodev, nodev, nodev, 0}, /* rp = 1 */
	{nodev, nodev, nodev, 0}, /* rf = 2 */
	{nodev, nodev, nodev, 0}, /* tm = 3 */
	{nodev, nodev, nodev, 0}, /* tc = 4 */
	{nodev, nodev, nodev, 0}, /* hs = 5 */
	{nodev, nodev, nodev, 0}, /* hp = 6 */
	{nodev, nodev, nodev, 0},	/* ht = 7 */
	{nodev, nodev, nodev, 0}, /* rl = 8 */
	{nulldev, nulldev, sdxstrategy, &sdxtab}, /* sdx = 9 */
	{0}
};

int	sdxread(), sdxwrite();
int	mmread(), mmwrite();
int	syopen(), syread(), sywrite(), sysioctl();

int bcm283x_pl011open();
int bcm283x_pl011close();
int bcm283x_pl011read();
int bcm283x_pl011write();
int bcm283x_pl011ioctl();
int bcm283x_pl011stop();

struct	cdevsw	cdevsw[] =
{
	{bcm283x_pl011open, bcm283x_pl011close, bcm283x_pl011read, bcm283x_pl011write, bcm283x_pl011ioctl, bcm283x_pl011stop, 0},	/* console = 0 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* pc = 1 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* lp = 2 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* dc = 3 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* dh = 4 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* dp = 5 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* dj = 6 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* dn = 7 */
	{nulldev, nulldev, mmread, mmwrite, nodev, nulldev, 0}, 	/* mem = 8 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0},	/* rk = 9 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* rf = 10 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* rp = 11 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* tm = 12 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* hs = 13 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* hp = 14 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0},	/* ht = 15 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* du = 16 */
	{syopen, nulldev, syread, sywrite, sysioctl, nulldev, 0}, /* tty = 17 */
	{nodev, nodev, nodev, nodev, nodev, nulldev, 0}, /* rl = 18 */
	{nulldev, nulldev, sdxread, sdxwrite, nodev, nulldev, 0}, /* sdx = 19 */
	{0}
};

extern caddr_t ttwrite(struct tty *tp);
int	ttyopen(), ttyclose(), ttread(), ttyinput(), ttstart();
struct	linesw	linesw[] =
{
	{ttyopen, nulldev, ttread, ttwrite, nodev, ttyinput, ttstart}, /* 0 */
	{0}
};
dev_t	rootdev	= makedev(9, 0);
dev_t	swapdev	= makedev(9, 1);
dev_t	pipedev = makedev(9, 0);
int	nldisp = 1;
daddr_t	swplo	= 1;
int	nswap	= 31256;

struct	buf	buf[NBUF];
struct	file	file[NFILE];
struct	inode	inode[NINODE];
struct	proc	proc[NPROC];
struct	text	text[NTEXT];
struct	buf	bfreelist;
struct	acct	acctbuf;
struct	inode	*acctp;
