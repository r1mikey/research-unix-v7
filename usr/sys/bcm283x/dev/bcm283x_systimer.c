#include "bcm283x_systimer.h"
#include "bcm283x_systimer_regs.h"

#include "bcm283x_io.h"                  /* ioread32, iowrite32              */
#include "bcm283x_irq.h"                 /* interrupt registration           */
#include "../arm1176jzfs.h"              /* read_cpsr                        */
#include "../trap.h"                     /* struct tf_regs_t                 */
#include "../../h/types.h"               /* u32                              */
#include "../../h/param.h"               /* NULL                             */
#include "../../h/clock.h"               /* clock                            */
#include "../../h/prf.h"                 /* panic                            */

#define BCM283X_SYSTMR_FREQUENCY         1000000  /* 1MHz                    */
#define BCM283X_SYSTMR_TICKS_PER_S       BCM283X_SYSTMR_FREQUENCY
#define BCM283X_SYSTMR_TICKS_PER_QUANTUM (BCM283X_SYSTMR_TICKS_PER_S / 10)

void udelay(u32 us)
{
  DMB;
  u32 now = ioread32(SYSTMR_CLO_REG);
  while (ioread32(SYSTMR_CLO_REG) - now < us);  /* one tick is one us */
  DMB;
}


static void clkintr(void *arg, struct tf_regs_t *tf)
{
  iowrite32(SYSTMR_C1_REG, ioread32(SYSTMR_CLO_REG) + BCM283X_SYSTMR_TICKS_PER_QUANTUM);
  iowrite32(SYSTMR_CS_REG, SYSTMR_CS_M1);
  clock(-1, tf->r13, tf->r1, read_cpsr(), tf->r0, (caddr_t)tf->r15, tf->cpsr);
}


void clkinit(void)
{
  u32 v;
  if (0 != bcm283x_register_timer_irq_handler(GPU_IRQ_SYSTIMER_1_INT, clkintr, NULL))
    panic("clkinit: failed to register IRQ handler");
  DMB;
  v = ioread32(SYSTMR_CLO_REG);
  DMB;
  iowrite32(SYSTMR_C1_REG, v - 1);
}
