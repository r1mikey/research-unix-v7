#ifndef BCM283X_SYSTIMER_H
#define BCM283X_SYSTIMER_H

#include "../kstdint.h"
#include "../kstddef.h"

extern void bcm283x_systimer_early_init(void);

extern void udelay(uint32_t us);
extern void mdelay(uint32_t ms);
extern uint64_t ticks(uint64_t *hz);
extern uint32_t tick_diff_us(uint64_t s, uint64_t e);
extern void clkinit(void);

#endif
