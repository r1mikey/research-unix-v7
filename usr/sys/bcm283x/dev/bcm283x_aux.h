#ifndef BCM283X_AUX_H
#define BCM283X_AUX_H

#include "../trap.h"
#include "../kstdint.h"
#include "../kstddef.h"

#define BCM283X_AUX_BLOCK_MINIUART 0
#define BCM283X_AUX_BLOCK_SPI1 1
#define BCM283X_AUX_BLOCK_SPI2 2

extern void bcm283x_aux_init(void);
extern void bcm283x_aux_deinit(void);

extern int bcm283x_aux_enable_block(uint32_t b);
extern int bcm283x_aux_disable_block(uint32_t b);

extern int bcm283x_register_aux_timer_irq_handler(uint32_t irqnum, void (*fn)(void*, struct tf_regs_t *), void *arg);
extern int bcm283x_register_aux_irq_handler(uint32_t irqnum, void (*fn)(void*), void *arg);
extern int bcm283x_deregister_aux_irq_handler(uint32_t irqnum);

#endif
