/* see exception.s */
#include "trap.h"
#include "kstddef.h"
#include "arm1176jzfs.h"
#include "undefined.h"
#include "dev/bcm283x_irq.h"

#include "../h/types.h"
#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "../h/reg.h"
#include "../h/sig.h"
#include "../h/slp.h"
#include "../h/prf.h"
#include "../h/machdep.h"


void trap_tail(struct tf_regs_t *tf, time_t syst)
{
  if (issig()) {
    psig();
  }

  curpri = setpri(u.u_procp);

  if (USERMODE(tf->cpsr) && runrun) {
    qswtch();
  }

  if (u.u_prof.pr_scale) {
    addupc((caddr_t)tf->r15, &u.u_prof, (int)(u.u_stime-syst));  /* CHECK */
  }

  if (u.u_fpsaved) {
    restfp(&u.u_fps);
  }
}


/*
 * UNDEF
 * Return Instruction: MOVS PC, R14_und
 * ARM R14_x: PC+4
 * Thumb R14_x: PC+2
 *
 * We need to identify the instruction at r15 as a VFP instruction and enable as needed.
 * fpsav and fprestore must check if enabled, and only operate if it is.
 * vfp shizzle when already enabled needs patching through to signals etc.
 */
void c_entry_und(struct tf_regs_t *tf)
{
  time_t syst;

  syst = u.u_stime;
  u.u_fpsaved = 0;

  if (!USERMODE(tf->cpsr)) {
    panic("undefined instruction in supervisor mode");
  }

  if (handle_undefined(tf)) {
    printf("Registers:\n"
      " r0  : 0x%x\n"
      " r1  : 0x%x\n"
      " r2  : 0x%x\n"
      " r3  : 0x%x\n"
      " r4  : 0x%x\n"
      " r5  : 0x%x\n"
      " r6  : 0x%x\n"
      " r7  : 0x%x\n"
      " r8  : 0x%x\n"
      " r9  : 0x%x\n"
      " r10 : 0x%x\n"
      " r11 : 0x%x\n"
      " r12 : 0x%x\n"
      " r13 : 0x%x\n"
      " r14 : 0x%x\n"
      " r15 : 0x%x\n"
      " cpsr: 0x%x\n",
      tf->r0, tf->r1, tf->r2, tf->r3, tf->r4, tf->r5, tf->r6, tf->r7,
      tf->r8, tf->r9, tf->r10, tf->r11, tf->r12, tf->r13, tf->r14, tf->r15,
      tf->cpsr);
    printf("Instruction: 0x%x\n", *((u32 *)tf->r15));
    psignal(u.u_procp, SIGINS);
  }

  trap_tail(tf, syst);
}


/*
 * Offsets of the user's registers relative to
 * the saved r0. See reg.h
 */
unsigned char regloc[] = {
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, RPS
};


#define SYS 0xef000000

void c_entry_swi(u32 sn, struct tf_regs_t *tf)
{
  int i;
  struct sysent *callp;
  time_t syst;
  u32 *a;
  int (*fetch)(caddr_t);

  syst = u.u_stime;
  u.u_fpsaved = 0;
  u.u_ar0 = (int *)tf;

  u.u_error = 0;
  tf->cpsr &= ~BIT(29);  /* clear the error bit returned to userland */
  a = (u32 *)tf->r14;  /* was pc, now lr */
  callp = &sysent[sn & 0x3f];

  if (callp == sysent) {  /* indirect */
    a = (u32 *)fuiword((caddr_t)a);
    i = fuword((caddr_t)a);
    a++;
    if (i > 077) {
      i = 077;  /* illegal */
    }
    callp = &sysent[i & 077];
    fetch = fuword;
  } else {  /* direct */
    fetch = fuiword;
  }

  for (i = 0; i < callp->sy_nrarg; ++i) {
    u.u_arg[i] = u.u_ar0[regloc[i]];
  }

  for ( ; i < callp->sy_narg; ++i) {
    u.u_arg[i] = (*fetch)((caddr_t)a++);
  }

  u.u_dirp = (caddr_t)u.u_arg[0];
  u.u_r.r_val1 = tf->r0;
  u.u_r.r_val2 = tf->r1;
  u.u_ap = u.u_arg;

  if (save(u.u_qsav)) {
    if (u.u_error == 0) {
      u.u_error = EINTR;
    }
  } else {
    (*callp->sy_call)();
  }

  if (u.u_error) {
    tf->cpsr |= BIT(29);
    tf->r0 = u.u_error;
  } else {
    tf->r0 = u.u_r.r_val1;
    tf->r1 = u.u_r.r_val2;
  }

  trap_tail(tf, syst);
}


static void c_entry_abt(struct tf_regs_t *tf, char t)
{
  u32 fsr;
  u32 far;

  /* see the ARMv6 ARM page 743 */
  if (t == 'D') {
    fsr = read_dfsr();
  } else {
    fsr = read_ifsr();
  }
  far = read_dfar();

  printf("%s abort in PID %u (%s) at 0x%x with PSR 0x%x\n", t == 'D' ? "Data" : "Prefetch", u.u_procp->p_pid, u.u_comm, tf->r15, tf->cpsr);
  printf("Registers:\n r0  : 0x%x\n r1  : 0x%x\n r2  : 0x%x\n r3  : 0x%x\n r4  : 0x%x\n r5  : 0x%x\n r6  : 0x%x\n r7  : 0x%x\n r8  : 0x%x\n r9  : 0x%x\n r10 : 0x%x\n r11 : 0x%x\n r12 : 0x%x\n r13 : 0x%x\n r14 : 0x%x\n r15 : 0x%x\n cpsr: 0x%x\n",
    tf->r0, tf->r1, tf->r2, tf->r3, tf->r4, tf->r5, tf->r6, tf->r7,
    tf->r8, tf->r9, tf->r10, tf->r11, tf->r12, tf->r13, tf->r14, tf->r15,
    tf->cpsr);

  /* ARM DDI 0100I B4.6 (Page 719) */
  switch ((((fsr & 0x400) >> 6) & 0x10) | (fsr & 0xf)) {
    case 0b00001:
      printf("%cABT: Alignment. FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b00000:
      printf("%cABT: PMSA - TLB miss (MPU). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b00011:
      printf("%cABT: Alignment (deprecated). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b00100:
      printf("%cABT: Instruction Cache Maintenance Operation Fault. FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01100:
      printf("%cABT: External Abort on Translation (1st level). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01110:
      printf("%cABT: External Abort on Translation (2nd level). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b00101:
      printf("%cABT: Translation (Section). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b00111:
      printf("%cABT: Translation (Page). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01001:
      printf("%cABT: Domain (Section). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01011:
      printf("%cABT: Domain (Page). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01101:
      printf("%cABT: Permission (Section). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01111:
      printf("%cABT: Permission (Page). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01000:
      printf("%cABT: Precise External Abort. FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b01010:
      printf("%cABT: External Abort, Precise (deprecated). FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    case 0b10100:
      printf("%cABT: TLB Lock. FSR: 0x%x.\n", t, fsr);
      break;
    case 0b11010:
      printf("%cABT: Coprocessor Data Abort. FSR: 0x%x.\n", t, fsr);
      break;
    case 0b10110:
      printf("%cABT: Imprecise External Abort. FSR: 0x%x.\n", t, fsr);
      break;
    case 0b11000:
      printf("%cABT: Parity Error Exception. FSR: 0x%x.\n", t, fsr);
      break;
    case 0b00010:
      printf("%cABT: Debug event. FSR: 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
    default:
      printf("%cABT: Unknown FSR 0x%x. FAR: 0x%x.\n", t, fsr, far);
      break;
  }
}


void c_entry_pabt(struct tf_regs_t *tf)
{
  c_entry_abt(tf, 'P');
  panic("prefetch abort");
}


void c_entry_dabt(struct tf_regs_t *tf)
{
  unsigned int osp;
  time_t syst;

  syst = u.u_stime;
  u.u_fpsaved = 0;

  if (!USERMODE(tf->cpsr)) {
    c_entry_abt(tf, 'D');
    panic("data abort in kernel mode");
  }

  osp = tf->r13;

  if (osp < (USERTOP - ctob(u.u_ssize))) {
    if (grow(osp)) {
      goto out;
    }
  }

  psignal(u.u_procp, SIGSEG);

out:
  trap_tail(tf, syst);
}


void c_entry_irq(struct tf_regs_t *tf)
{
  time_t syst;
  syst = u.u_stime;
  u.u_fpsaved = 0;
  irqc(tf);
  trap_tail(tf, syst);
}
