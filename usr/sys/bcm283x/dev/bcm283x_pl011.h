#ifndef BCM283X_PL011_H
#define BCM283X_PL011_H

#include "../kstddef.h"

#define REQUIRED_PLL011_CLOCK_RATE_MHZ 3000000  /* 3MHz */
extern void bcm283x_uart_early_init(void);

#endif
