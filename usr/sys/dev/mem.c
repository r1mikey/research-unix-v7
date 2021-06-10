#
/*
 */

/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/RATHOLE
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/seg.h"

#include "../bcm283x/arm1176jzfs.h"
#include "../bcm283x/page_tables.h"

/* XXX: prototypes */
extern int spl0(void);                                          /* <asm> */
extern int spl7(void);                                          /* <asm> */
extern void splx(int s);                                        /* <asm> */
extern int fuibyte(caddr_t addr);                               /* <asm> */
extern int passc(int c);                                        /* sys/subr.c */
extern int cpass(void);                                         /* sys/subr.c */
extern void panic(char *s);                                     /* sys/prf.c */
/* XXX: end prototypes */

physadr	ka6;		/* 11/40 KISA6; 11/45 KDSA6 -- UNUSED */

extern char __kernelspace_start[];
extern char __kernelspace_end[];

extern char __copypage_dst[];

static void mmread_eof(dev_t dev)
{
	/* de nada */
}


static void mmread_kmem(dev_t dev)
{
	int s;
	int c;

	if (u.u_offset < (off_t)__kernelspace_start || u.u_offset >= (off_t)__kernelspace_end) {
		u.u_error = EINVAL;
		return;
	}

	do {
		s = spl7();
		if ((c = fuibyte((caddr_t)u.u_offset)) < 0)
			u.u_error = ENXIO;
		splx(s);
	} while (u.u_error == 0 && passc(c) >= 0);
}


static void mmread_physmem(dev_t dev)
{
	int s;
	int c;
	off_t on;
	long bn;
	off_t fetch_addr;

	do {
		bn = u.u_offset >> PGSHIFT;
		on = u.u_offset & (PGSZ - 1);
		fetch_addr = (off_t)__copypage_dst + on;

		s = spl7();
		if (page_is_mapped(btoc((u32)__copypage_dst))) {
			dcacheciva((u32)__copypage_dst, ((u32)__copypage_dst) + PGSZ - 1);
			tlbimva((u32)__copypage_dst, 0);
		}
		setup_one_page_mapping(bn, btoc(__copypage_dst), 0x0000061f);  /* R/XN */
		DMB;
		if ((c = fuibyte((caddr_t)fetch_addr)) < 0)
			u.u_error = ENXIO;
		DMB;
		dcacheiva((u32)__copypage_dst, ((u32)__copypage_dst) + PGSZ - 1);
		tlbimva((u32)__copypage_dst, 0);
		setup_one_page_mapping(0, btoc(__copypage_dst), 0);
		splx(s);
	} while (u.u_error == 0 && passc(c) >= 0);
}


void mmread(dev_t dev)
{
	switch (minor(dev)) {
		case 0:  /* physical memory */
			mmread_physmem(dev);
			break;
		case 1:  /* kernel memory */
			mmread_kmem(dev);
			break;
		case 2:  /* eof/rathole */
			mmread_eof(dev);
			break;
		default:
			u.u_error = ENODEV;
			break;
	}
}

void mmwrite(dev_t dev)
{
	int c, bn, on;
	int a, d;

	if(minor(dev) == 2) {
		u.u_count = 0;
		return;
	}
	panic("mmwrite needs to be imnplemented");
	for(;;) {
		bn = u.u_offset >> 6;
		on = u.u_offset & 077;
		if ((c=cpass())<0 || u.u_error!=0)
			break;
		a = UISA->r[0];
		d = UISD->r[0];
		spl7();
		UISA->r[0] = bn;
		UISD->r[0] = 077406;
		if(minor(dev) == 1)
			UISA->r[0] = (ka6-6)->r[(bn>>7)&07] + (bn & 0177);
		if (suibyte((caddr_t)on, c) < 0)
			u.u_error = ENXIO;
		UISA->r[0] = a;
		UISD->r[0] = d;
		spl0();
	}
}
