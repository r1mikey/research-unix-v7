#ifndef __V7_SYS_CLOCK_H
#define __V7_SYS_CLOCK_H

#include "types.h"

extern void clock(dev_t dev, unsigned int sp, unsigned int r1, unsigned int nps, unsigned int r0, caddr_t pc, unsigned int ps);

#endif
