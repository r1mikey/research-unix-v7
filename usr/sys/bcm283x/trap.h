#ifndef BCM283X_TRAP_H
#define BCM283X_TRAP_H

#include "../h/types.h"

struct tf_regs_t {
  u32 r0;    /* user-mode r0  */
  u32 r1;    /* user-mode r1  */
  u32 r2;    /* user-mode r2  */
  u32 r3;    /* user-mode r3  */
  u32 r4;    /* user-mode r4  */
  u32 r5;    /* user-mode r5  */
  u32 r6;    /* user-mode r6  */
  u32 r7;    /* user-mode r7  */
  u32 r8;    /* user-mode r8  */
  u32 r9;    /* user-mode r9  */
  u32 r10;   /* user-mode r10 */
  u32 r11;   /* user-mode r11 */
  u32 r12;   /* user-mode fp  */
  u32 r13;   /* user-mode sp  */
  u32 r14;   /* user-mode lr  */
  u32 r15;   /* user-mode pc  */
  u32 cpsr;  /* user-mode psr */
  u32 pad;
};

#endif
