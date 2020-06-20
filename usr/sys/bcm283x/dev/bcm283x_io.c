#include "bcm283x_io.h"



u8 ioread8(caddr_t addr)
{
  return (*((volatile u8 *)addr)) & 0xff;
}


u16 ioread16(caddr_t addr)
{
  return (*((volatile u16 *)addr)) & 0xffff;
}


u32 ioread32(caddr_t addr)
{
  return *((volatile u32 *)addr);
}


void iowrite8(caddr_t addr, u8 v)
{
  *((volatile u8 *)addr) = v;
}


void iowrite16(caddr_t addr, u16 v)
{
  *((volatile u16 *)addr) = v;
}


void iowrite32(caddr_t addr, u32 v)
{
  *((volatile u32 *)addr) = v;
}


void iosetbits8(caddr_t addr, u8 b)
{
  *((volatile u8 *)addr) = ((*((volatile u8 *)addr)) & 0xff) | b;
}


void iosetbits16(caddr_t addr, u16 b)
{
  *((volatile u16 *)addr) = ((*((volatile u16 *)addr)) & 0xffff) | b;
}


void iosetbits32(caddr_t addr, u32 b)
{
  *((volatile u32 *)addr) = (*((volatile u32 *)addr)) | b;
}


void ioclrbits8(caddr_t addr, u8 b)
{
  *((volatile u8 *)addr) = ((*((volatile u8 *)addr)) & 0xff) & ~b;
}


void ioclrbits16(caddr_t addr, u16 b)
{
  *((volatile u16 *)addr) = ((*((volatile u16 *)addr)) & 0xffff) & ~b;
}


void ioclrbits32(caddr_t addr, u32 b)
{
  *((volatile u32 *)addr) = (*((volatile u32 *)addr)) & ~b;
}


int iotstbits8(caddr_t addr, u8 b)
{
  return (((*((volatile u8 *)addr)) & 0xff) & b) == b;
}


int iotstbits16(caddr_t addr, u16 b)
{
  return (((*((volatile u16 *)addr)) & 0xffff) & b) == b;
}


int iotstbits32(caddr_t addr, u32 b)
{
  return ((*((volatile u32 *)addr)) & b) == b;
}
