#ifndef BCM283X_IO_H
#define BCM283X_IO_H

#include "../../h/types.h"


extern u8 ioread8(caddr_t addr);
extern u16 ioread16(caddr_t addr);
extern u32 ioread32(caddr_t addr);

extern void iowrite8(caddr_t addr, u8 v);
extern void iowrite16(caddr_t addr, u16 v);
extern void iowrite32(caddr_t addr, u32 v);

extern void iosetbits8(caddr_t addr, u8 b);
extern void iosetbits16(caddr_t addr, u16 b);
extern void iosetbits32(caddr_t addr, u32 b);

extern void ioclrbits8(caddr_t addr, u8 b);
extern void ioclrbits16(caddr_t addr, u16 b);
extern void ioclrbits32(caddr_t addr, u32 b);

extern int iotstbits8(caddr_t addr, u8 b);
extern int iotstbits16(caddr_t addr, u16 b);
extern int iotstbits32(caddr_t addr, u32 b);

#endif
