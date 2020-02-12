#ifndef BCM283X_MBOX_H
#define BCM283X_MBOX_H

#include "../kstdint.h"
#include "../kstddef.h"

extern int bcm283x_mbox_get_arm_memory(uint32_t *v);
extern int bcm283x_mbox_set_uart_clock(uint32_t hz, uint32_t *new_hz);
extern int bcm283x_mbox_get_sdcard_clock(uint32_t *hz);

#endif
