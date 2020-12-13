/*
 * This file is based, in part, on sys/arm/arm/vfp.c from FreeBSD.
 */
/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2014 Ian Lepore <ian@freebsd.org>
 * Copyright (c) 2012 Mark Tinguely
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "vfp.h"

#include "kstddef.h"
#include "arm1176jzfs.h"

#include "../h/types.h"
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "../h/reg.h"
#include "../h/prf.h"
#include "../h/sig.h"

#define COPROCESSOR_10 (0x3 << 20)
#define COPROCESSOR_11 (0x3 << 22)
#define VFP_COPROCESSORS (COPROCESSOR_10|COPROCESSOR_11)

u32 initial_fpscr = 0x02000000 | 0x01000000;  /* default NaN enable, flush to zero enabled */
static int vfp_exists;
static int is_d32;  /* If true the VFP unit has 32 double registers, otherwise it has 16 */


void vfp_init(void)
{
  u32 fpsid;
  u32 tmp;
  /* u32 coproc; */
  u32 vfp_arch;

  write_cpacr(read_cpacr() | VFP_COPROCESSORS);

  fpsid = read_fpsid();       /* read the vfp system id */
  (void)read_fpexc();         /* read the vfp exception reg - necessary? */

  if (fpsid & 0x00800000) {
    printf("FPSID indicates VFPSID_HARDSOFT_IMP\n");
    return;
  }

  vfp_exists = 1;
  is_d32 = 0;
  /* PCPU_SET(vfpsid, fpsid); */    /* save the fpsid, but nobody uses it */
  vfp_arch = (fpsid & 0x000f0000) >> 16;

  if (vfp_arch >= 0x2) {  /* VFP arch 3 */
    tmp = read_mvfr0();
    /* PCPU_SET(vfpmvfr0, tmp); */ /* nobody uses it */

    if ((tmp & 0x0000000f) == 0x2) {  /* VFP 64 bit media support */
      is_d32 = 1;
    }

    tmp = read_mvfr1();
    /* PCPU_SET(vfpmvfr1, tmp); */ /* nobody uses it */

    /* XXX: only on CPU 0 */
    if ((tmp & 0x0000000f) == 0x1) {  /* NEON denormals arithmetic support */
      initial_fpscr &= ~0x01000000;
    }
  }
}


void vfp_store(int disable_vfp)
{
  u32 fpexc;

  fpexc = read_fpexc();

  if (!(fpexc & 0x40000000)) {  /* VFP enabled? */
    return;
  }

  u.u_fps.u_fpexec = fpexc;
  u.u_fps.u_fpscr = read_fpscr();

  if (fpexc & 0x80000000) {  /* exception v1 v2 */
    u.u_fps.u_fpinst = read_fpinst();
    if (fpexc & 0x10000000) {  /* FPINST2 valid */
      u.u_fps.u_fpinst2 = read_fpinst2();
    }
    fpexc &= ~0x80000000;
  }

  vfp_saveregs(u.u_fps.u_fpregs, is_d32);

  if (disable_vfp) {
    write_fpexc(fpexc & ~0x40000000);
  }
}


void vfp_discard(void)
{
  u32 tmp;
  tmp = read_fpexc();
  if (tmp & 0x40000000) {  /* only disable if enabled */
    write_fpexc(tmp & ~0x40000000);
  }
}


void vfp_atexec(void)
{
  int i;

  vfp_discard();
  u.u_fpsaved = 0;
  u.u_fps.u_fpscr = initial_fpscr;
  u.u_fps.u_fpexec = 0;
  u.u_fps.u_fpinst = 0;
  u.u_fps.u_fpinst2 = 0;

  for (i = 0; i < 32; ++i) {
    u.u_fps.u_fpregs[i] = 0;
  }
}


int vfp_bounce(void)
{
  u32 fpexc;
  int s;

  if (!vfp_exists) {
    return 1;
  }

  s = spl7();

  fpexc = read_fpexc();
  if (fpexc & 0x40000000) {  /* if already enabled */
    write_fpexc(fpexc & ~(0x80000000|0x10000000));  /* clean any exceptions */

    /* signal the process */
    splx(s);

    if (fpexc & 0x80000000) {  /* if we have an exception, arrange for a SIGFPE */
      /* there's a lot more information that can be passed here */
      psignal(u.u_procp, SIGFPT);
      return 0;
    }

    return 1;
  }

  u.u_fps.u_fpexec |= 0x40000000;  /* VFP enable */
  write_fpexc(u.u_fps.u_fpexec);
  if (u.u_fps.u_fpexec & 0x80000000) {  /* exception v1 v2 */
    write_fpinst(u.u_fps.u_fpinst);
    if (u.u_fps.u_fpexec & 0x10000000) {  /* FPINST2 valid */
      write_fpinst2(u.u_fps.u_fpinst2);
    }
  }
  write_fpscr(u.u_fps.u_fpscr);
  vfp_loadregs(u.u_fps.u_fpregs, is_d32);
  u.u_fps.u_fpexec = read_fpexc();
  splx(s);
  return 0;
}


void savfp(void * x)
{
  if (x != &u.u_fps) {
    panic("bad savfp");
  }

  vfp_store(1);
}


void restfp(void *x)
{
  u32 fpexc;

  if (x != &u.u_fps) {
    panic("bad restfp");
  }

  /* nothing to do, just disable and allow the exception handler to turn it on */
  fpexc = read_fpexc();
  if (fpexc & 0x40000000) {
    write_fpexc(fpexc & ~0x40000000);
  }
}
