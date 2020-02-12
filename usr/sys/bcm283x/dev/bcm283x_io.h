#ifndef BCM283X_IO_H
#define BCM283X_IO_H

#include "../kstdint.h"
#include "../kstddef.h"


extern uint8_t ioread8(devaddr_t addr);
extern uint16_t ioread16(devaddr_t addr);
extern uint32_t ioread32(devaddr_t addr);

extern void iowrite8(devaddr_t addr, uint8_t v);
extern void iowrite16(devaddr_t addr, uint16_t v);
extern void iowrite32(devaddr_t addr, uint32_t v);

extern void iosetbits8(devaddr_t addr, uint8_t b);
extern void iosetbits16(devaddr_t addr, uint16_t b);
extern void iosetbits32(devaddr_t addr, uint32_t b);

extern void ioclrbits8(devaddr_t addr, uint8_t b);
extern void ioclrbits16(devaddr_t addr, uint16_t b);
extern void ioclrbits32(devaddr_t addr, uint32_t b);

extern int iotstbits8(devaddr_t addr, uint8_t b);
extern int iotstbits16(devaddr_t addr, uint16_t b);
extern int iotstbits32(devaddr_t addr, uint32_t b);

#endif
