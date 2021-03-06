#include "bcm283x_machdep.h"

#include "dev/bcm283x_io.h"
#include "dev/bcm283x_systimer.h"
#include "dev/bcm283x_pl011.h"
#include "dev/bcm283x_irq.h"
#include "dev/bcm283x_mbox.h"
#include "dev/bcm283x_gpio.h"
#include "kstddef.h"
#include "arm1176jzfs.h"
#include "page_tables.h"
#include "vfp.h"

#include "../h/types.h"
#include <stdint.h>

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/seg.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/reg.h"
#include "../h/prf.h"
#include "../h/malloc.h"
#include "../h/sig.h"
#include "../h/ureg.h"

int	maxmem;			/* actual max memory per process */
int	cputype;		/* type of cpu =40, 45, or 70 -- UNUSED */

extern char __clearpage_dst[];
extern char __copypage_src[];
extern char __copypage_dst[];

extern int sdx_init(u32 hz);

/* startup - add memory not used by the kernel to the coremap - called by main */
/*   -- set up the user I/O segment (I think this is a pointer to the per-process user page) - needed? maybe... we could keep one aside for the user page, gets complicated */
/* sysphys - a system call of some kind that maps a physical address into a user segment! */
/* clkstart - determine which clock to use and start it - called by main */
/* sendsig - arranges for a process to exevute a signal (from what I can see) */

/*
 * icode is the hex bootstrap program executed in user mode to bring up the
 * system.
 *
 * Calls the exec syscall to execute /etc/init - as simple as it gets.
 */
int icode[] = {
               /*     .syntax unified                              */
               /*     .arm                                         */
               /*     .section .text                               */
               /*                                                  */
               /*     .global _start                               */
               /*     _start                                       */
  0xe59fe028,  /* 00:     ldr     lr, =indirect                    */
  0xef000000,  /* 04:     swi     #0                               */
  0xeafffffe,  /* 08:     b       .                                */
               /*                                                  */
               /* .p2align 2                                       */
  0x6374652f,  /* 0c: arg0: .asciz "/etc/init" @ (/   e   t   c  ) */
  0x696e692f,  /* 10:                          @ (/   i   n   i  ) */
  0x00000074,  /* 14:                          @ (t   NUL NUL NUL) */
               /*                                                  */
               /*     .p2align 2                                   */
               /*     arg1:                                        */
  0x0000000c,  /* 18:     .word   arg0                             */
  0x00000000,  /* 1c:     .word   0                                */
               /*                                                  */
               /*     .p2align 2                                   */
               /*     args:                                        */
  0x0000000b,  /* 20:     .word   11                               */
  0x0000000c,  /* 24:     .word   arg0                             */
  0x00000018,  /* 28:     .word   arg1                             */
               /*                                                  */
               /*     .p2align 2                                   */
               /*     indirect:                                    */
  0x00000020,  /* 2c:     .word   args                             */
  0x0000002c,  /* 30: @ que?                                       */
};
int szicode = sizeof(icode);


void startup(void)
{
  u32 virtpg;
  u32 mem;
  u32 npag;
  u32 sd_base_hz;
  int ret;

  sd_base_hz = 0;

  dcacheciva(0, 0x00ffffff);
  icacheiva(0, 0x00ffffff);

  for (virtpg = 0; virtpg < 4095; ++virtpg) {
    setup_one_page_mapping(0, virtpg, 0);
    tlbimva(virtpg << 12, 0);
  }

  DSB;

  (void)bcm283x_mbox_set_uart_clock(REQUIRED_PLL011_CLOCK_RATE_MHZ, (u32 *)0);
  bcm283x_gpio_setup_for_pl011();

  bcm283x_uart_early_init();      /* panic is now available */
  bcm283x_init_irq();             /* IRQ registration is now possible */

  /* power off doesn't seem to work as advertised, at least on Pi1 */
  (void)bcm283x_mbox_sdcard_power(0);
  udelay(5);
  if (0 != bcm283x_mbox_sdcard_power(1))
    panic("sd card power on");
  udelay(5);

  ret = bcm283x_mbox_get_sdcard_clock(&sd_base_hz);
  if (ret || !sd_base_hz) {
    sd_base_hz = 200000000;
    printf("startup/sdx: using default eMMC base clock\n");
  }

  if (0 != sdx_init(sd_base_hz))  /* read the UNIX partition details from the SD card */
    panic("sdx_init");

  mem = 0;
  if (0 != bcm283x_mbox_get_arm_memory(&mem))
    panic("bcm283x_mbox_get_arm_memory");
  npag = (mem - _first_available_phys_addr) >> 12;
  if (npag > 0x7fff) {
    npag = 0x7fff;  /* size is a short, which is limiting on modern hardware */
  }

  mfree(coremap, npag, _first_available_phys_addr >> 12);

  if (npag > MAXMEM) {
    maxmem = MAXMEM;
  } else {
    maxmem = npag;
  }

  printf("mem = %D\n", npag * PGSZ);

  /* unit size is 512 bytes - first arg is the number of blocks and the second is the first block */
  /* same restrictions apply as do for core memory (size has 15 usable bits) */
  mfree(swapmap, nswap, swplo);
  swplo--;  /* this becomes zero-indexed at this point */

  vfp_init();
  u.u_fps.u_fpscr = initial_fpscr;
}


void clkstart(void)
{
  clkinit();
}


/*
 * - Figure out bytes needed, grow the stack as needed
 * - Push {fp, pc} to the stack
 * - Set fp to point to the frame pointer
 * - Align the stack as needed
 * - Save the PSR to the stack
 * - Modify the SP to reflect this
 * - Modify the PC to point to the trampoline
 * - A signal handler needs no arguments in this version of UNIX, so no need
 *   to pass those, the trampoline will save r0-r3
 */
void sendsig(caddr_t p, int signo)
{
  u32 n;

  n = u.u_ar0[SP] - (3 * NBPW);
  grow(n);

  suword((caddr_t)(n + (2 * NBPW)), u.u_ar0[PC]);
  suword((caddr_t)(n + (1 * NBPW)), u.u_ar0[FP]);
  suword((caddr_t)(n + (0 * NBPW)), u.u_ar0[RPS]);
  u.u_ar0[FP] = n;
  u.u_ar0[SP] = n;

  /* u.u_ar0[RPS] &= ~TBIT -- we do not have a trace bit! */
  u.u_ar0[PC] = (u32)p;
}


void sysphys()
{
  u.u_error = EINVAL;
  /* panic("sysphys not implemented"); */
}


/*
 * mapfree
 *
 * PDP 11/70 helper to unmap UNIBUS resources. We will not need this
 */
void mapfree(void *bp)
{

}


void display(void)
{
}


/*
 * we evaluate first uisa, then uisd.
 * top half contains the number of blocks (1 or 2, depending on the number in this "segment"), bottom half the attributes
 * mapping is done from va 0
 * mapping is done physically relative to u.u_procp
 * when separate text is used, program text is mapped from the text cache
 *
 * The first page in the user allocation is the udot page, already mapped elsewhere.
 */
int estabur(unsigned int nt, unsigned int nd, unsigned int ns, int sep, int xrw)
{
  int uisa0;
  int uisa1;

  if (nt + nd + ns + USIZE > maxmem) {
    goto out;
  }

  /* redundant */
  if (nt > 128 || nd > 128 || ns > 128) {
    goto out;
  }

  uisa0 = u.u_uisa[0];
  uisa1 = u.u_uisa[1];

  u.u_uisa[0] = (nt << 16) | (nd << 8) | ns;
  if (sep) {
    u.u_uisa[1] = ((xrw | TX) << 16) | (RW << 8) | (RW | ED);
  } else {
    u.u_uisa[1] = ((RW | TX) << 16) | ((RW | TX) << 8) | (RW | ED);
  }

  u.u_uisa[2] = u.u_uisa[0] != uisa0 || u.u_uisa[1] != uisa1;

  sureg();
  return 0;

out:
  u.u_error = ENOMEM;
  return -1;
}


static unsigned int setup_ureg(unsigned int srcpg, unsigned int dstpg, unsigned int n, unsigned int a)
{
  u32 attr;

  if (!n) {
    return dstpg;
  }

  if (a & TX) {
    if (a & RW) {
      attr = 0x00000c5e;
    } else {
      attr = 0x00000e1e;
    }
  } else {
    if (a & RW || a & ED) {
      attr = 0x00000c5f;
    } else {
      if (a) {
        attr = 0x00000e1f;
      } else {
        attr = 0x00000000;
      }
    }
  }

  while (n) {
    setup_one_page_mapping(srcpg++, dstpg++, attr);
    --n;
  }

  return dstpg;
}


void sureg(void)
{
  unsigned int nt;
  unsigned int ta;
  unsigned int nd;
  unsigned int da;
  unsigned int ns;
  unsigned int sa;
  unsigned int cb;
  struct text *tp;

  u32 virtpg;
  u32 physpg;

  u32 oasid;
  u32 nasid;

  u32 maddr;
  u32 msize;

  u32 dirty;
  u32 realised;

  ns = u.u_uisa[0] & 0xff;
  nd = (u.u_uisa[0] >> 8) & 0xff;
  nt = (u.u_uisa[0] >> 16) & 0xff;
  sa = u.u_uisa[1] & 0xff;
  da = (u.u_uisa[1] >> 8) & 0xff;
  ta = (u.u_uisa[1] >> 16) & 0xff;
  cb = 128 - nt - nd - ns;
  dirty = u.u_uisa[2] ? 1 : 0;
  realised = u.u_uisa[3] == 42;
  /* printf("pid: %d, comm %s, text: %u, data: %u, clear: %u, stack: %u, dirty: %u, realised: %d\n", u.u_procp->p_pid, u.u_comm, nt, nd, cb, ns, dirty, realised); */

  oasid = read_asid();
  nasid = u.u_procp - &proc[0];

  if (oasid != nasid) {
    /* printf("oldasid: %u -> new proc %d, new asid %u\n", oasid, u.u_procp->p_pid, nasid); */
    write_asid(nasid);
    flush_entire_btc();
  } else {
    if (dirty && realised) {
      flush_current_pagetable(1, 1, nasid);
    }
  }

  virtpg = 0;

  if ((tp = u.u_procp->p_textp)) {
    physpg = tp->x_caddr;  /* really? */
    maddr = virtpg * PGSZ;
    msize = nt * PGSZ;
    virtpg = setup_ureg(physpg, virtpg, nt, ta);
    physpg = u.u_procp->p_addr + 1;  /* first page is udot */
  } else {
    physpg = u.u_procp->p_addr + 1;  /* first page is udot */
    maddr = virtpg * PGSZ;
    msize = nt * PGSZ;
    virtpg = setup_ureg(physpg, virtpg, nt, ta);
    physpg += nt;
  }
  if (dirty && msize)
    dcacheiva(maddr, (maddr + msize) - 1);

  maddr = virtpg * PGSZ;
  msize = nd * PGSZ;
  virtpg += setup_ureg(physpg, virtpg, nd, da);
  physpg += nd;
  if (dirty && msize)
    dcacheiva(maddr, (maddr + msize) - 1);

  setup_ureg(0, virtpg, cb, 0);  /* unmap unused virtual address space */
  maddr = (USERTOP) - (ns * PGSZ);
  msize = ns * PGSZ;
  setup_ureg(physpg, (USERTOP/PGSZ) - ns, ns, sa);  /* map the stack */
  if (dirty && ns)
    dcacheiva(maddr, (maddr + msize) - 1);

  if (dirty) {
    if (realised)
      tlbiasid(nasid);
    u.u_uisa[2] = 0;  /* no longer dirty */
    DMB;
  }

  if (!realised)
    u.u_uisa[3] = 42;  /* realised */
}


void copyseg(int from, int to)
{
  int s;
  unsigned int copypage_dst;
  unsigned int copypage_src;
  unsigned int dstpg;
  unsigned int srcpg;

  copypage_dst = (unsigned int)__copypage_dst;
  copypage_src = (unsigned int)__copypage_src;
  dstpg = copypage_dst >> 12;
  srcpg = copypage_src >> 12;

  s = spl7();

  if (page_is_mapped(from)) {
    dcacheiva(copypage_src, (copypage_src + PGSZ) - 1);
    tlbimva(copypage_src, 0);
  }

  if (page_is_mapped(to)) {
    dcacheiva(copypage_dst, (copypage_dst + PGSZ) - 1);
    tlbimva(copypage_dst, 0);
  }

  setup_one_page_mapping(from, srcpg, 0x0000061f);  /* R/XN   */
  setup_one_page_mapping(to, dstpg, 0x0000045f);    /* R/W/XN */

  __copyseg_helper(copypage_src, copypage_dst);

  dcacheiva(copypage_src, (copypage_src + PGSZ) - 1);
  dcacheciva(copypage_dst, (copypage_dst + PGSZ) - 1);
  tlbimva(copypage_src, 0);
  tlbimva(copypage_dst, 0);
  DMB;
  splx(s);
}


void clearseg(int a)
{
  int s;
  unsigned int clearpage_dst;
  unsigned int dstpg;

  clearpage_dst = (unsigned int)__clearpage_dst;
  dstpg = clearpage_dst >> 12;
  s = spl7();

  if (page_is_mapped(a)) {
    dcacheiva(clearpage_dst, (clearpage_dst + PGSZ) - 1);
    tlbimva(clearpage_dst, 0);
  }

  setup_one_page_mapping(a, dstpg, 0x0000045f);

  __clearseg_helper(clearpage_dst);

  dcacheciva(clearpage_dst, (clearpage_dst + PGSZ) - 1);
  tlbimva(clearpage_dst, 0);
  DMB;

  splx(s);
}


int copyout(const unsigned int *src, unsigned int *dst, unsigned int sz)
{
  if (((u32)src & 3) == 0 && ((u32)dst & 3) == 0 && sz % 4 == 0) {
    while (sz) {
      *dst++ = *src++;
      sz -= NBPW;
    }
  } else {
    unsigned char *s;
    unsigned char *d;

    s = (unsigned char *)src;
    d = (unsigned char *)dst;

    while (sz) {
      *d++ = *s++;
      sz -= 1;
    }
  }

  return 0;
}


int copyin(const unsigned int *src, unsigned int *dst, unsigned int sz)
{
  return copyout(src, dst, sz);
}


int copyiin(const unsigned int *src, unsigned int *dst, unsigned int sz)
{
  return copyout(src, dst, sz);
}


int copyiout(const unsigned int *src, unsigned int *dst, unsigned int sz)
{
  return copyout(src, dst, sz);
}
