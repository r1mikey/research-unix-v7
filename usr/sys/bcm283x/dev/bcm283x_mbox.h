#ifndef BCM283X_MBOX_H
#define BCM283X_MBOX_H

#include "../kstddef.h"
#include "../../h/types.h"

extern int bcm283x_mbox_get_arm_memory(u32 *v);
extern int bcm283x_mbox_set_uart_clock(u32 hz, u32 *new_hz);
extern int bcm283x_mbox_get_sdcard_clock(u32 *hz);

extern int bcm283x_mbox_sdcard_power(u32 onoff);

#endif
