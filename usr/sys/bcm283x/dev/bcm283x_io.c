#include "bcm283x_io.h"



uint8_t ioread8(devaddr_t addr)
{
  return (*((volatile uint8_t *)addr)) & 0xff;
}


uint16_t ioread16(devaddr_t addr)
{
  return (*((volatile uint16_t *)addr)) & 0xffff;
}


uint32_t ioread32(devaddr_t addr)
{
  return *((volatile uint32_t *)addr);
}


void iowrite8(devaddr_t addr, uint8_t v)
{
  *((volatile uint8_t *)addr) = v;
}


void iowrite16(devaddr_t addr, uint16_t v)
{
  *((volatile uint16_t *)addr) = v;
}


void iowrite32(devaddr_t addr, uint32_t v)
{
  *((volatile uint32_t *)addr) = v;
}


void iosetbits8(devaddr_t addr, uint8_t b)
{
  *((volatile uint8_t *)addr) = ((*((volatile uint8_t *)addr)) & 0xff) | b;
}


void iosetbits16(devaddr_t addr, uint16_t b)
{
  *((volatile uint16_t *)addr) = ((*((volatile uint16_t *)addr)) & 0xffff) | b;
}


void iosetbits32(devaddr_t addr, uint32_t b)
{
  *((volatile uint32_t *)addr) = (*((volatile uint32_t *)addr)) | b;
}


void ioclrbits8(devaddr_t addr, uint8_t b)
{
  *((volatile uint8_t *)addr) = ((*((volatile uint8_t *)addr)) & 0xff) & ~b;
}


void ioclrbits16(devaddr_t addr, uint16_t b)
{
  *((volatile uint16_t *)addr) = ((*((volatile uint16_t *)addr)) & 0xffff) & ~b;
}


void ioclrbits32(devaddr_t addr, uint32_t b)
{
  *((volatile uint32_t *)addr) = (*((volatile uint32_t *)addr)) & ~b;
}


int iotstbits8(devaddr_t addr, uint8_t b)
{
  return (((*((volatile uint8_t *)addr)) & 0xff) & b) == b;
}


int iotstbits16(devaddr_t addr, uint16_t b)
{
  return (((*((volatile uint16_t *)addr)) & 0xffff) & b) == b;
}


int iotstbits32(devaddr_t addr, uint32_t b)
{
  return ((*((volatile uint32_t *)addr)) & b) == b;
}
