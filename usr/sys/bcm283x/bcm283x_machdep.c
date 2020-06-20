#include "bcm283x_machdep.h"

#include "dev/bcm283x_io.h"
#include "dev/bcm283x_systimer.h"
#include "dev/bcm283x_pl011.h"
#include "dev/bcm283x_irq.h"
#include "dev/bcm283x_mbox.h"
#include "dev/bcm283x_sdcard.h"
#include "kstddef.h"
#include "arm1176jzfs.h"
#include "page_tables.h"
#include "vfp.h"

#include <stdint.h>

#undef NULL

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/seg.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/reg.h"


extern void printf(const char *fmt, ...);
extern void mfree(struct map *mp, int size, int a);
extern void wakeup(caddr_t chan);
extern int grow(uint32_t sp);

extern uint32_t read_cpuid(void);
extern uint32_t read_ttbr0(void);
extern uint32_t read_curcpu(void);

/* icode and szicode - init stub to call the exec syscall and execute /etc/init - simple as it gets */
/* startup - add memory not used by the kernel to the coremap - called by main */
/*   -- set up the user I/O segment (I think this is a pointer to the per-process user page) - needed? maybe... we could keep one aside for the user page, gets complicated */
/* sysphys - a system call of some kind that maps a physical address into a user segment! */
/* clkstart - determine which clock to use and start it - called by main */
/* sendsig - arranges for a process to exevute a signal (from what I can see) */
/* mapalloc - PDP 11/70 helper to map UNIBUS resources - we will not need this */
/* mapfree - corresponding PDP 11/70 helper to unmap UNIBUS resources - we will not need this */

/*
 * Icode is the hex bootstrap
 * program executed in user mode
 * to bring up the system.
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


static void _bcm283x_handle_gpu_irq(void * arg)
{
  /* printf("_bcm283x_handle_gpu_irq: fired\n"); */
}


/* extern void timeout(void (*fun)(void *), void *arg, int tim); */

void startup(void)
{
  uint32_t virtpg;
  uint32_t mem;
  uint32_t npag;
  int s;

  pre_page_table_modification();

  for (virtpg = 0; virtpg < 4095; ++virtpg) {
    setup_one_page_mapping(0, virtpg, 0);
  }

  post_page_table_modification();

  bcm283x_uart_early_init();      /* panic is now available */
  bcm283x_systimer_early_init();  /* delay is now available */
  bcm283x_init_irq();             /* IRQ registration is now possible */

  bcm283x_register_irq_handler(CORE_IRQ_GPU_INT_N(read_curcpu()), _bcm283x_handle_gpu_irq, NULL);

  bcm283x_sdcard_init();          /* read the UNIX partition details from the SD card */

  mem = 0;
  bcm283x_mbox_get_arm_memory(&mem);
  npag = (mem - _first_available_phys_addr) >> 12;
  if (npag > 0x7fff) {
    npag = 0x7fff;  /* size is a short, which is limiting on moderns hardware */
  }

  mfree(coremap, npag, _first_available_phys_addr >> 12);

  if (npag > MAXMEM) {
    maxmem = MAXMEM;
  } else {
    maxmem = npag;
  }

  printf("mem = %D\n", npag * 4096);

  /* unit size is 512 bytes - first arg is the number of blocks and the second is the first block */
  /* same restrictions apply as do for core memory (size has 15 usable bits) */
  mfree(swapmap, nswap, swplo);
  swplo--;  /* this becomes zero-indexed at this point */

  vfp_init();
  u.u_fps.u_fpscr = initial_fpscr;
#if 0
  printf("&udot == 0x%x-0x%x\n", &u, ((uint32_t)&u) + sizeof(u));
  printf("sizeof(struct user) == %u\n", sizeof(struct user));

  for (s = 0; s < NOFILE; ++s) {
    u.u_ofile[s] = NULL;
    u.u_pofile[s] = 0;
  }
#endif
}


void clkstart(void)
{
  clkinit();
}


/*
 * PDP11 VERSION
 * =============
 * Ensure we have enough stack to stash variables
 * Stash the old PC and PSR
 * Set the new stack pointer
 * Clear the T bit from the PSR (what is this?)
 * Set the PC to the new function
 *
 * ARM VERSION
 * ===========
 * Figure out bytes needed, grow the stack as needed
 * Push {fp, pc} to the stack
 * Set fp to point to the frame pointer
 * Align the stack as needed
 * Save the PSR to the stack
 * Modify the SP to reflect this
 * Modify the PC to point to the trampoline
 * A signal handler needs no arguments in this version of UNIX, so no need to pass those, the trampoline will save r0-r3
 */
void sendsig(devaddr_t p, int signo)
{
  unsigned int n;

  n = u.u_ar0[SP] - (3 * NBPW);
  grow(n);

  suword((uint32_t)n + (2 * NBPW), u.u_ar0[PC]);
  suword((uint32_t)n + (1 * NBPW), u.u_ar0[FP]);
  suword((uint32_t)n + (0 * NBPW), u.u_ar0[RPS]);
  u.u_ar0[FP] = n;
  u.u_ar0[SP] = n;

  /* u.u_ar0[RPS] &= ~TBIT -- we do not have a trace bit! */
  u.u_ar0[PC] = (unsigned int)p;
}


void sysphys()
{
  u.u_error = EINVAL;
  /* panic("sysphys not implemented"); */
}


void mapfree(void *bp)
{
  /* nothing to do - we don't have a UNIBUS ;) */
}


void display(void)
{
}


extern void sureg(void);

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
  if (nt + nd + ns + USIZE > maxmem) {
    goto out;
  }

  /* redundant */
  if (nt > 128 || nd > 128 || ns > 128) {
    goto out;
  }

  u.u_uisa[0] = (nt << 16) | (nd << 8) | ns;
  if (sep) {
    u.u_uisa[1] = ((xrw | TX) << 16) | (RW << 8) | (RW | ED);
  } else {
    u.u_uisa[1] = ((RW | TX) << 16) | ((RW | TX) << 8) | (RW | ED);
  }

  sureg();
  return 0;

out:
  u.u_error = ENOMEM;
  return -1;
}


static unsigned int setup_ureg(unsigned int srcpg, unsigned int dstpg, unsigned int n, unsigned int a)
{
  uint32_t attr;

  if (!n) {
    return dstpg;
  }

  if (a & TX) {
    if (a & RW) {
      attr = 0x0000045e;
    } else {
      attr = 0x0000061e;
    }
  } else {
    if (a & RW || a & ED) {
      attr = 0x0000045f;
    } else {
      if (a) {
        attr = 0x0000061f;
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
  unsigned int taddr;
  unsigned int daddr;
  struct text *tp;

  uint32_t virtpg;
  uint32_t physpg;
  unsigned int n;
  unsigned int a;
  unsigned int i;

  ns = u.u_uisa[0] & 0xff;
  nd = (u.u_uisa[0] >> 8) & 0xff;
  nt = (u.u_uisa[0] >> 16) & 0xff;
  sa = u.u_uisa[1] & 0xff;
  da = (u.u_uisa[1] >> 8) & 0xff;
  ta = (u.u_uisa[1] >> 16) & 0xff;
  cb = 128 - nt - nd - ns;
  /* printf("text: %u, data: %u, clear: %u, stack: %u\n", nt, nd, cb, ns); */

  pre_page_table_modification();

  for (i = 0; i < (MAXMEM - 1); ++i) {
    setup_one_page_mapping(0, i, 0);
  }

  /* XXX: this is broken */
#if 0
  for (i = 0; i < 4096; ++i) {
    setup_one_page_mapping(0, BCM283X_MAX_VIRT_PAGES - i - 1, 0);
  }
#endif

  virtpg = 0;

  if ((tp = u.u_procp->p_textp)) {
    physpg = tp->x_caddr;  /* really? */
    virtpg = setup_ureg(physpg, virtpg, nt, ta);
    physpg = u.u_procp->p_addr + 1;  /* first page is udot */
  } else {
    physpg = u.u_procp->p_addr + 1;  /* first page is udot */
    virtpg = setup_ureg(physpg, virtpg, nt, ta);
    physpg += nt;
  }

  virtpg += setup_ureg(physpg, virtpg, nd, da);
  physpg += nd;

  setup_ureg(0, virtpg, cb, 0);  /* unmap unused virtual address space */
  setup_ureg(physpg, (USERTOP/PGSZ) - ns, ns, sa);  /* map the stack */

  post_page_table_modification();
  do_arm1176jzfs_isb();
  do_invalidate_icache();
}


int copyout(const unsigned int *src, unsigned int *dst, unsigned int sz)
{
  if ((u32)src & 3 == 0 && (u32)dst & 3 == 0 && sz % 4 == 0) {
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
