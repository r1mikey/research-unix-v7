#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/filsys.h"
#include "../h/file.h"
#include "../h/conf.h"
#include "../h/inode.h"
#include "../h/reg.h"
#include "../h/acct.h"

/* XXX: prototypes */
extern void wakeup(caddr_t chan);                               /* sys/slp.c */
extern void iput(struct inode *ip);                             /* sys/iget.c */
extern void plock(struct inode *ip);                            /* sys/pipe.c */
extern void xrele(struct inode *ip);                            /* sys/text.c */
extern void printf(const char *fmt, ...);                       /* sys/prf.c */
extern struct filsys * getfs(dev_t dev);                        /* sys/alloc.c */
extern struct inode * namei(int (*func)(), int flag);           /* sys/nami.c */
extern int uchar(void);                                         /* sys/nami.c */
/* XXX: end prototypes */

/*
 * Convert a user supplied
 * file descriptor into a pointer
 * to a file structure.
 * Only task is to check range
 * of the descriptor.
 */
struct file * getf(int f)
{
	register struct file *fp;

	if(0 <= f && f < NOFILE) {
		fp = u.u_ofile[f];
		if(fp != NULL)
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * Internal form of close.
 * Decrement reference count on
 * file structure.
 * Also make sure the pipe protocol
 * does not constipate.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 * Call device handler on last close.
 */
void closef(struct file *fp)
{
	register struct inode *ip;
	int flag, mode;
	dev_t dev;
	register int (*cfunc)();

	if(fp == NULL)
		return;
	if (fp->f_count > 1) {
		fp->f_count--;
		return;
	}
	ip = fp->f_inode;
	flag = fp->f_flag;
	dev = (dev_t)ip->i_un.i_rdev;
	mode = ip->i_mode;

	plock(ip);
	fp->f_count = 0;
	if(flag & FPIPE) {
		ip->i_mode &= ~(IREAD|IWRITE);
		wakeup((caddr_t)ip+1);
		wakeup((caddr_t)ip+2);
	}
	iput(ip);

	switch(mode&IFMT) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		break;

	case IFBLK:
		cfunc = bdevsw[major(dev)].d_close;
		break;
	default:
		return;
	}

	for(fp=file; fp < &file[NFILE]; fp++)
		if (fp->f_count && fp->f_inode==ip)
			return;
	(*cfunc)(dev, flag);
}

/*
 * openi called to allow handler
 * of special files to initialize and
 * validate before actual IO.
 */
void openi(struct inode *ip, int rw)
{
	dev_t dev;
	register u16 maj;

	dev = (dev_t)ip->i_un.i_rdev;
	maj = major(dev);
	switch(ip->i_mode&IFMT) {

	case IFCHR:
		if(maj >= nchrdev)
			goto bad;
		(*cdevsw[maj].d_open)(dev, rw);
		break;

	case IFBLK:
		if(maj >= nblkdev)
			goto bad;
		(*bdevsw[maj].d_open)(dev, rw);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the
 * read-only status of the file
 * system is checked.
 * Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select
 * the owner/group/other fields.
 * The super user is granted all
 * permissions.
 */
int access(struct inode *ip, int mode)
{
	int m;

	m = mode;
	if(m == IWRITE) {
		if(getfs(ip->i_dev)->s_ronly != 0) {
			u.u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&ITEXT)		/* try to free text */
			xrele(ip);
		if(ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
	}
	if(u.u_uid == 0)
		return(0);
	if(u.u_uid != ip->i_uid) {
		m >>= 3;
		if(u.u_gid != ip->i_gid)
			m >>= 3;
	}
	if((ip->i_mode&m) != 0)
		return(0);

	u.u_error = EACCES;
	return(1);
}

/*
 * Test if the current user is the
 * super user.
 */
int suser(void)
{

	if(u.u_uid == 0) {
		u.u_acflag |= ASU;
		return(1);
	}
	u.u_error = EPERM;
	return(0);
}

/*
 * Look up a pathname and test if
 * the resultant inode is owned by the
 * current user.
 * If not, try for super-user.
 * If permission is granted,
 * return inode pointer.
 */
struct inode * owner()
{
	struct inode *ip;

	ip = namei(uchar, 0);
	if(ip == NULL)
		return(NULL);
	if(u.u_uid == ip->i_uid)
		return(ip);
	if(suser())
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Allocate a user file descriptor.
 */
int ufalloc(void)
{
	int i;

	for(i=0; i<NOFILE; i++)
		if(u.u_ofile[i] == NULL) {
			u.u_r.r_val1 = i;
			u.u_pofile[i] = 0;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor
 * and a file structure.
 * Initialize the descriptor
 * to point at the file structure.
 *
 * no file -- if there are no available
 * 	file structures.
 */
struct file * falloc(void)
{
	struct file *fp;
	int i;

	i = ufalloc();
	if(i < 0)
		return(NULL);
	for(fp = &file[0]; fp < &file[NFILE]; fp++)
		if(fp->f_count == 0) {
			u.u_ofile[i] = fp;
			fp->f_count++;
			fp->f_offset = 0;
			return(fp);
		}
	printf("no file\n");
	u.u_error = ENFILE;
	return(NULL);
}
