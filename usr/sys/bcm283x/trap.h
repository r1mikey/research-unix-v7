#ifndef BCM283X_TRAP_H
#define BCM283X_TRAP_H

#include "kstdint.h"

struct tf_regs_t {
  uint32_t r0;    /* user-mode r0  */
  uint32_t r1;    /* user-mode r1  */
  uint32_t r2;    /* user-mode r2  */
  uint32_t r3;    /* user-mode r3  */
  uint32_t r4;    /* user-mode r4  */
  uint32_t r5;    /* user-mode r5  */
  uint32_t r6;    /* user-mode r6  */
  uint32_t r7;    /* user-mode r7  */
  uint32_t r8;    /* user-mode r8  */
  uint32_t r9;    /* user-mode r9  */
  uint32_t r10;   /* user-mode r10 */
  uint32_t r11;   /* user-mode r11 */
  uint32_t r12;   /* user-mode fp  */
  uint32_t r13;   /* user-mode sp  */
  uint32_t r14;   /* user-mode lr  */
  uint32_t r15;   /* user-mode pc  */
  uint32_t cpsr;  /* user-mode psr */
  uint32_t pad;
};

/* like label_t I think? */
struct cpu_context_t {
  uint32_t r4;    /* XX */
  uint32_t r5;    /* XX */
  uint32_t r6;    /* XX */
  uint32_t r7;    /* XX */
  uint32_t r8;    /* XX */
  uint32_t r9;    /* XX */
  uint32_t sl;    /* r10 */
  uint32_t fp;    /* r11 */
  uint32_t sp;    /* r13 */
  uint32_t pc;    /* r15 */
};

#endif
