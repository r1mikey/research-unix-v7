#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/reg.h"
#include "../h/file.h"
#include "../h/inode.h"

/* XXX: prototypes */
extern void readp(struct file *fp);                             /* sys/pipe.c */
extern void writep(struct file *fp);                            /* sys/pipe.c */
extern void plock(struct inode *ip);                            /* sys/pipe.c */
extern void readi(struct inode *ip);                            /* sys/rdwri.c */
extern void writei(struct inode *ip);                           /* sys/rdwri.c */
extern void prele(struct inode *ip);                            /* sys/pipe.c */
extern int access(struct inode *ip, int mode);                  /* sys/fio.c */
extern void itrunc(struct inode *ip);                           /* sys/iget.c */
extern void openi(struct inode *ip, int rw);                    /* sys/fio.c */
extern void iput(struct inode *ip);                             /* sys/iget.c */
extern void closef(struct file *fp);                            /* sys/fio.c */
extern int suser(void);                                         /* sys/fio.c */
extern void wdir(struct inode *ip);                             /* sys/iget.c */
extern int uchar(void);                                         /* sys/nami.c */
extern struct inode * namei(int (*func)(), int flag);           /* sys/nami.c */
extern struct inode * maknode(int mode);
extern struct file * getf(int f);                               /* sys/fio.c */
extern struct file * falloc(void);

/* forward declarations */
void rdwr(int mode);
void open1(struct inode *ip, int mode, int trf);
/* XXX: end prototypes */

/*
 * read system call
 */
void read(void)
{
	rdwr(FREAD);
}

/*
 * write system call
 */
void write(void)
{
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi, writei, or pipe code.
 */
void rdwr(int mode)
{
	struct file *fp;
	struct inode *ip;
	struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if((fp->f_flag&mode) == 0) {
		u.u_error = EBADF;
		return;
	}
	u.u_base = (caddr_t)uap->cbuf;
	u.u_count = uap->count;
	u.u_segflg = 0;
	if((fp->f_flag&FPIPE) != 0) {
		if(mode == FREAD)
			readp(fp);
		else
			writep(fp);
	} else {
		ip = fp->f_inode;
		u.u_offset = fp->f_offset;
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			plock(ip);
		if(mode == FREAD)
			readi(ip);
		else
			writei(ip);
		if((ip->i_mode&(IFCHR&IFBLK)) == 0)
			prele(ip);
		fp->f_offset += uap->count-u.u_count;
	}
	u.u_r.r_val1 = uap->count-u.u_count;
}

/*
 * open system call
 */
void open(void)
{
	struct inode *ip;
	struct a {
		char	*fname;
		int	rwmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0);
	if(ip == NULL)
		return;
	open1(ip, ++uap->rwmode, 0);
}

/*
 * creat system call
 */
void creat(void)
{
	struct inode *ip;
	struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 1);
	if(ip == NULL) {
		if(u.u_error)
			return;
		ip = maknode(uap->fmode&07777&(~ISVTX));
		if (ip==NULL)
			return;
		open1(ip, FWRITE, 2);
	} else
		open1(ip, FWRITE, 1);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
void open1(struct inode *ip, int mode, int trf)
{
	struct file *fp;
	int i;

	if(trf != 2) {
		if(mode&FREAD)
			access(ip, IREAD);
		if(mode&FWRITE) {
			access(ip, IWRITE);
			if((ip->i_mode&IFMT) == IFDIR)
				u.u_error = EISDIR;
		}
	}
	if(u.u_error)
		goto out;
	if(trf == 1)
		itrunc(ip);
	prele(ip);
	if ((fp = falloc()) == NULL)
		goto out;
	fp->f_flag = mode&(FREAD|FWRITE);
	fp->f_inode = ip;
	i = u.u_r.r_val1;
	openi(ip, mode&FWRITE);
	if(u.u_error == 0)
		return;
	u.u_ofile[i] = NULL;
	fp->f_count--;

out:
	iput(ip);
}

/*
 * close system call
 */
void close(void)
{
	struct file *fp;
	struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	u.u_ofile[uap->fdes] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
void seek(void)
{
	struct file *fp;
	struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if(fp->f_flag&FPIPE) {
		u.u_error = ESPIPE;
		return;
	}
	if(uap->sbase == 1)
		uap->off += fp->f_offset;
	else if(uap->sbase == 2)
		uap->off += fp->f_inode->i_size;
	fp->f_offset = uap->off;
	u.u_r.r_off = uap->off;
}

/*
 * link system call
 */
void link(void)
{
	struct inode *ip, *xp;
	struct a {
		char	*target;
		char	*linkname;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(uchar, 0);
	if(ip == NULL)
		return;
	if((ip->i_mode&IFMT)==IFDIR && !suser())
		goto out;
	/*
	 * Unlock to avoid possibly hanging the namei.
	 * Sadly, this means races. (Suppose someone
	 * deletes the file in the meantime?)
	 * Nor can it be locked again later
	 * because then there will be deadly
	 * embraces.
	 */
	prele(ip);
	u.u_dirp = (caddr_t)uap->linkname;
	xp = namei(uchar, 1);
	if(xp != NULL) {
		u.u_error = EEXIST;
		iput(xp);
		goto out;
	}
	if (u.u_error)
		goto out;
	if(u.u_pdir->i_dev != ip->i_dev) {
		iput(u.u_pdir);
		u.u_error = EXDEV;
		goto out;
	}
	wdir(ip);
	if (u.u_error==0) {
		ip->i_nlink++;
		ip->i_flag |= ICHG;
	}

out:
	iput(ip);
}

/*
 * mknod system call
 */
void mknod(void)
{
	struct inode *ip;
	struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;

	uap = (struct a *)u.u_ap;
	if(suser()) {
		ip = namei(uchar, 1);
		if(ip != NULL) {
			u.u_error = EEXIST;
			goto out;
		}
	}
	if(u.u_error)
		return;
	ip = maknode(uap->fmode);
	if (ip == NULL)
		return;
	ip->i_un.i_rdev = (dev_t)uap->dev;

out:
	iput(ip);
}

/*
 * access system call
 */
void saccess(void)
{
	int svuid, svgid;
	struct inode *ip;
	struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ip = namei(uchar, 0);
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			access(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			access(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			access(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}
