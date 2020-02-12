#include "bcm283x_aux.h"

#include "bcm283x_io.h"
#include "bcm283x_irq.h"
#include "../kstddef.h"
#include "../arm1176jzfs.h"

/* #include "irq.h" */

#define AUX_OFFSET                    0x00215000
#define AUX_BASE                      ((_bcm283x_iobase) + (AUX_OFFSET))

#define AUX_AUXIRQ_REG_OFFSET         0x00000000
#define AUX_AUXENB_REG_OFFSET         0x00000004

#define AUX_AUXIRQ_REG                ((AUX_BASE) + (AUX_AUXIRQ_REG_OFFSET))
#define AUX_AUXENB_REG                ((AUX_BASE) + (AUX_AUXENB_REG_OFFSET))

#define AUXIRQ_MINIUART               0x00000001
#define AUXIRQ_SPI1                   0x00000002
#define AUXIRQ_SPI2                   0x00000004

#define AUXENB_MINIUART               0x00000001
#define AUXENB_SPI1                   0x00000002
#define AUXENB_SPI2                   0x00000004



extern devaddr_t _bcm283x_iobase;          /* peripheral base address */

static irq_handler_t irq_handlers[3];

extern void panic(const char *s) __attribute__((noreturn));



int bcm283x_register_aux_timer_irq_handler(uint32_t irqnum, void (*fn)(void*, struct tf_regs_t *), void *arg)
{
  if (irqnum > 2) {
    return -1;
  }

  if (!fn) {
    return -1;
  }

  irq_handlers[irqnum].fn = NULL;
  irq_handlers[irqnum].tfn = fn;
  irq_handlers[irqnum].arg = arg;
  return 0;
}


int bcm283x_register_aux_irq_handler(uint32_t irqnum, void (*fn)(void*), void *arg)
{
  if (irqnum > 2) {
    return -1;
  }

  if (!fn) {
    return -1;
  }

  irq_handlers[irqnum].fn = fn;
  irq_handlers[irqnum].tfn = NULL;
  irq_handlers[irqnum].arg = arg;
  return 0;
}


int bcm283x_deregister_aux_irq_handler(uint32_t irqnum)
{
  if (irqnum > 2) {
    return -1;
  }

  irq_handlers[irqnum].fn = NULL;
  irq_handlers[irqnum].tfn = NULL;
  irq_handlers[irqnum].arg = NULL;
  return 0;
}


int bcm283x_aux_enable_block(uint32_t b)
{
  if (b > 2) {
    return -1;
  }

  iosetbits32(AUX_AUXENB_REG, BIT(b));
  return 0;
}


int bcm283x_aux_disable_block(uint32_t b)
{
  if (b > 2) {
    return -1;
  }

  ioclrbits32(AUX_AUXENB_REG, BIT(b));
  return 0;
}


static void bcm283x_aux_intr(void *arg, struct tf_regs_t *tf)
{
  uint32_t i;
  uint32_t st;
  irq_handler_t *handler;

  do {} while (&arg != &arg);
  st = ioread32(AUX_AUXIRQ_REG);

  for (i = 0; i < 3; ++i) {
    if ((st >> i) & 0x1) {
      handler = &irq_handlers[i];

      if (!handler->fn && !handler->tfn) {
        panic("missing aux IRQ handler");
      }

      if (handler->fn) {
        handler->fn(handler->arg);
      } else {
        handler->tfn(handler->arg, tf);
      }
    }
  }
}


#if 0
void bcm283x_aux_init()
{
  int rc;

  iowrite32(SYSTMR_C1_REG, ioread32(SYSTMR_CLO_REG) - 1);

}
#endif


void bcm283x_aux_init(void)
{
  uint32_t i;
  int rc;
  irq_handler_t *handler;

  iowrite32(AUX_AUXENB_REG, 0);

  for (i = 0; i < 3; ++i) {
    handler = &irq_handlers[i];
    handler->tfn = NULL;
    handler->fn = NULL;
    handler->arg = NULL;
  }

  if (0 != (rc = bcm283x_register_timer_irq_handler(GPU_IRQ_AUX_INT, bcm283x_aux_intr, NULL))) {
    panic("bcm283x_aux_init: failed to register IRQ handler");
  }
}


void bcm283x_aux_deinit(void)
{
  uint32_t i;
  int rc;
  irq_handler_t *handler;

  iowrite32(AUX_AUXENB_REG, 0);

  for (i = 0; i < 3; ++i) {
    handler = &irq_handlers[i];
    handler->tfn = NULL;
    handler->fn = NULL;
    handler->arg = NULL;
  }

  if (0 != (rc = bcm283x_deregister_irq_handler(GPU_IRQ_AUX_INT))) {
    panic("bcm283x_aux_deinit: failed to deregister IRQ handler");
  }
}
