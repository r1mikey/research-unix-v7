#ifndef BCM283X_SYSTIMER_H
#define BCM283X_SYSTIMER_H

#include "../kstddef.h"
#include "../../h/types.h"

extern void bcm283x_systimer_early_init(void);

extern void udelay(u32 us);
extern void mdelay(u32 ms);
extern u64 ticks(u64 *hz);
extern u32 tick_diff_us(u64 s, u64 e);
extern void clkinit(void);

#endif
