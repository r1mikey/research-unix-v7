#include "undefined.h"

#include "trap.h"
#include "kstddef.h"
#include "arm1176jzfs.h"
#include "vfp.h"

#undef NULL

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "../h/reg.h"

extern void printf(const char *fmt, ...);                       /* sys/prf.c */


static void print_undefined(struct tf_regs_t *tf, uint32_t instr)
{
  printf("Undefined instruction 0x%x at 0x%x, PSR 0x%x, PID %d, COMM %s\n",
    instr, tf->r15, tf->cpsr, u.u_procp->p_pid, u.u_comm);
}


int handle_undefined(struct tf_regs_t *tf)
{
  uint32_t instr;
  uint8_t copro;

  instr = *((uint32_t *)tf->r15);  /* so very dodgy */

  if (instr & BIT(27)) {
    copro = (instr >> 8) & 0xf;
  } else if ((instr & 0xfe000000) == 0xf2000000 || (instr & 0xff100000) == 0xf4000000) {
    copro = 10;  /* VFP */
  } else {
    print_undefined(tf, instr);
    return 1;
  }

  if (copro != 10 && copro != 11) {
    print_undefined(tf, instr);
    return 1;
  }

  if (vfp_bounce()) {
    print_undefined(tf, instr);
    return 1;
  }

  return 0;
}
