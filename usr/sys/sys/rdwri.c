#include "../h/param.h"
#include "../h/systm.h"
#include "../h/inode.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"

/* XXX: prototypes */
extern void brelse(struct buf *bp);                             /* dev/bio.c */
extern void bdwrite(struct buf *bp);                            /* dev/bio.c */
extern void clrbuf(struct buf *bp);                             /* dev/bio.c */
extern int copyout(const unsigned int *src, unsigned int *dst, unsigned int sz);
extern int copyin(const unsigned int *src, unsigned int *dst, unsigned int sz);
extern int copyiout(const unsigned int *src, unsigned int *dst, unsigned int sz);
extern int copyiin(const unsigned int *src, unsigned int *dst, unsigned int sz);
extern int cpass(void);                                         /* sys/subr.c */
extern int passc(int c);                                        /* sys/subr.c */
extern struct buf * getblk(dev_t dev, daddr_t blkno);           /* dev/bio.c */
extern struct buf * bread(dev_t dev, daddr_t blkno);            /* dev/bio.c */
extern struct buf * geteblk(void);
extern daddr_t bmap(struct inode *ip, daddr_t bn, int rwflg);
extern struct buf * breada(dev_t dev, daddr_t blkno, daddr_t rablkno);

/* forward declarations */
void iomove(caddr_t cp, int n, int flag);
/* XXX: end prototypes */


/*
 * Return the logical maximum
 * of the 2 arguments.
 */
unsigned int max(unsigned int a, unsigned int b)
{
	if(a > b)
		return(a);
	return(b);
}

/*
 * Return the logical minimum
 * of the 2 arguments.
 */
unsigned int min(unsigned int a, unsigned int b)
{
	if(a < b)
		return(a);
	return(b);
}

/*
 * Read the file corresponding to
 * the inode pointed at by the argument.
 * The actual read arguments are found
 * in the variables:
 *	u_base		core address for destination
 *	u_offset	byte offset in file
 *	u_count		number of bytes to read
 *	u_segflg	read to kernel/user/user I
 */
void readi(struct inode *ip)
{
	struct buf *bp;
	dev_t dev;
	daddr_t lbn, bn;
	off_t diff;
	int on, n;
	int type;

	if(u.u_count == 0)
		return;
	if(u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	ip->i_flag |= IACC;
	dev = (dev_t)ip->i_un.i_rdev;
	type = ip->i_mode&IFMT;
	if (type==IFCHR) {
		(*cdevsw[major(dev)].d_read)(dev);
		return;
	}

	do {
		lbn = bn = u.u_offset >> BSHIFT;
		on = u.u_offset & BMASK;
		n = min((unsigned)(BSIZE-on), u.u_count);
		if (type!=IFBLK) {
			diff = ip->i_size - u.u_offset;
			if(diff <= 0)
				return;
			if(diff < n)
				n = diff;
			bn = bmap(ip, bn, B_READ);
			if(u.u_error)
				return;
			dev = ip->i_dev;
		} else
			rablock = bn+1;
		if ((long)bn<0) {
			bp = geteblk();
			clrbuf(bp);
		} else if (ip->i_un.i_lastr+1==lbn)
			bp = breada(dev, bn, rablock);
		else
			bp = bread(dev, bn);
		ip->i_un.i_lastr = lbn;
		n = min((unsigned)n, BSIZE-bp->b_resid);
		if (n!=0)
			iomove(bp->b_un.b_addr+on, n, B_READ);
		brelse(bp);
	} while(u.u_error==0 && u.u_count!=0 && n>0);
}

/*
 * Write the file corresponding to
 * the inode pointed at by the argument.
 * The actual write arguments are found
 * in the variables:
 *	u_base		core address for source
 *	u_offset	byte offset in file
 *	u_count		number of bytes to write
 *	u_segflg	write to kernel/user/user I
 */
void writei(struct inode *ip)
{
	struct buf *bp;
	dev_t dev;
	daddr_t bn;
	int n, on;
	int type;

	if(u.u_offset < 0) {
		u.u_error = EINVAL;
		return;
	}
	dev = (dev_t)ip->i_un.i_rdev;
	type = ip->i_mode&IFMT;
	if (type==IFCHR) {
		ip->i_flag |= IUPD|ICHG;
		(*cdevsw[major(dev)].d_write)(dev);
		return;
	}
	if (u.u_count == 0)
		return;

	do {
		bn = u.u_offset >> BSHIFT;
		on = u.u_offset & BMASK;
		n = min((unsigned)(BSIZE-on), u.u_count);
		if (type!=IFBLK) {
			bn = bmap(ip, bn, B_WRITE);
			if((long)bn<0)
				return;
			dev = ip->i_dev;
		}
		if(n == BSIZE) 
			bp = getblk(dev, bn);
		else
			bp = bread(dev, bn);
		iomove(bp->b_un.b_addr+on, n, B_WRITE);
		if(u.u_error != 0)
			brelse(bp);
		else
			bdwrite(bp);
		if(u.u_offset > ip->i_size &&
		   (type==IFDIR || type==IFREG))
			ip->i_size = u.u_offset;
		ip->i_flag |= IUPD|ICHG;
	} while(u.u_error==0 && u.u_count!=0);
}

/*
 * Move n bytes at byte location
 * &bp->b_un.b_addr[o] to/from (flag) the
 * user/kernel (u.segflg) area starting at u.base.
 * Update all the arguments by the number
 * of bytes moved.
 *
 * There are 2 algorithms,
 * if source address, dest address and count
 * are all even in a user copy,
 * then the machine language copyin/copyout
 * is called.
 * If not, its done byte-by-byte with
 * cpass and passc.
 */
void iomove(caddr_t cp, int n, int flag)
{
	int t;

	if (n==0)
		return;
	if(u.u_segflg != 1 &&
	  (n&(NBPW-1)) == 0 &&
	  ((int)cp&(NBPW-1)) == 0 &&
	  ((int)u.u_base&(NBPW-1)) == 0) {
		if (flag==B_WRITE)
			if (u.u_segflg==0)
				t = copyin((unsigned int *)u.u_base, (unsigned int *)(caddr_t)cp, n);
			else
				t = copyiin((unsigned int *)u.u_base, (unsigned int *)(caddr_t)cp, n);
		else
			if (u.u_segflg==0)
				t = copyout((unsigned int *)(caddr_t)cp, (unsigned int *)u.u_base, n);
			else
				t = copyiout((unsigned int *)(caddr_t)cp, (unsigned int *)u.u_base, n);
		if (t) {
			u.u_error = EFAULT;
			return;
		}
		u.u_base += n;
		u.u_offset += n;
		u.u_count -= n;
		return;
	}
	if (flag==B_WRITE) {
		do {
			if ((t = cpass()) < 0)
				return;
			*cp++ = t;
		} while (--n);
	} else
		do {
			if(passc(*cp++) < 0)
				return;
		} while (--n);
}
