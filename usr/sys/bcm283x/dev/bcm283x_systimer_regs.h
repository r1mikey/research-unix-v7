#ifndef BCM283X_SYSTIMER_REGS_H
#define BCM283X_SYSTIMER_REGS_H

#include "../bcm283x_machdep.h"       /* for _bcm283x_iobase                 */

#define SYSTMR_OFFSET                 0x00003000
#define SYSTMR_BASE                   ((_bcm283x_iobase) + (SYSTMR_OFFSET))

#define SYSTMR_CS_REG_OFFSET          0x00000000  /* control and status      */
#define SYSTMR_CLO_REG_OFFSET         0x00000004  /* free-running low bits   */
#define SYSTMR_CHI_REG_OFFSET         0x00000008  /* free-running high bits  */
#define SYSTMR_C0_REG_OFFSET          0x0000000c  /* used by GPU             */
#define SYSTMR_C1_REG_OFFSET          0x00000010  /* sched timer             */
#define SYSTMR_C2_REG_OFFSET          0x00000014  /* used by GPU             */
#define SYSTMR_C3_REG_OFFSET          0x00000018  /* profiling timer?        */

#define SYSTMR_CS_REG                 ((SYSTMR_BASE) + (SYSTMR_CS_REG_OFFSET))
#define SYSTMR_CLO_REG                ((SYSTMR_BASE) + (SYSTMR_CLO_REG_OFFSET))
#define SYSTMR_CHI_REG                ((SYSTMR_BASE) + (SYSTMR_CHI_REG_OFFSET))
#define SYSTMR_C0_REG                 ((SYSTMR_BASE) + (SYSTMR_C0_REG_OFFSET))
#define SYSTMR_C1_REG                 ((SYSTMR_BASE) + (SYSTMR_C1_REG_OFFSET))
#define SYSTMR_C2_REG                 ((SYSTMR_BASE) + (SYSTMR_C2_REG_OFFSET))
#define SYSTMR_C3_REG                 ((SYSTMR_BASE) + (SYSTMR_C3_REG_OFFSET))

#define SYSTMR_CS_M0                  0x00000001  /* system timer match 0    */
#define SYSTMR_CS_M1                  0x00000002  /* system timer match 1    */
#define SYSTMR_CS_M2                  0x00000004  /* system timer match 2    */
#define SYSTMR_CS_M3                  0x00000008  /* system timer match 3    */

#endif
