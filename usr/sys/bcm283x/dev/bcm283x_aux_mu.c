#include "bcm283x_aux_mu.h"
#include "bcm283x_gpio.h"
#include "bcm283x_aux.h"
#include "bcm283x_io.h"
#include "../arm1176jzfs.h"

#define AUX_OFFSET                    0x00215000
#define AUX_BASE                      ((_bcm283x_iobase) + (AUX_OFFSET))

#define AUX_MU_IO_REG_OFFSET          0x00000040
#define AUX_MU_IER_REG_OFFSET         0x00000044
#define AUX_MU_IIR_REG_OFFSET         0x00000048
#define AUX_MU_LCR_REG_OFFSET         0x0000004c
#define AUX_MU_MCR_REG_OFFSET         0x00000050
#define AUX_MU_LSR_REG_OFFSET         0x00000054
#define AUX_MU_MSR_REG_OFFSET         0x00000058
#define AUX_MU_SCRATCH_REG_OFFSET     0x0000005c
#define AUX_MU_CNTL_REG_OFFSET        0x00000060
#define AUX_MU_STAT_REG_OFFSET        0x00000064
#define AUX_MU_BAUD_REG_OFFSET        0x00000068

#define AUX_MU_IO_REG                 ((AUX_BASE) + (AUX_MU_IO_REG_OFFSET))
#define AUX_MU_IER_REG                ((AUX_BASE) + (AUX_MU_IER_REG_OFFSET))
#define AUX_MU_IIR_REG                ((AUX_BASE) + (AUX_MU_IIR_REG_OFFSET))
#define AUX_MU_LCR_REG                ((AUX_BASE) + (AUX_MU_LCR_REG_OFFSET))
#define AUX_MU_MCR_REG                ((AUX_BASE) + (AUX_MU_MCR_REG_OFFSET))
#define AUX_MU_LSR_REG                ((AUX_BASE) + (AUX_MU_LSR_REG_OFFSET))
#define AUX_MU_MSR_REG                ((AUX_BASE) + (AUX_MU_MSR_REG_OFFSET))
#define AUX_MU_SCRATCH_REG            ((AUX_BASE) + (AUX_MU_SCRATCH_REG_OFFSET))
#define AUX_MU_CNTL_REG               ((AUX_BASE) + (AUX_MU_CNTL_REG_OFFSET))
#define AUX_MU_STAT_REG               ((AUX_BASE) + (AUX_MU_STAT_REG_OFFSET))
#define AUX_MU_BAUD_REG               ((AUX_BASE) + (AUX_MU_BAUD_REG_OFFSET))


extern devaddr_t _bcm283x_iobase;          /* peripheral base address */


void bcm283x_uart_early_init(void)
{
  bcm283x_aux_enable_block(BCM283X_AUX_BLOCK_MINIUART);

  iowrite32(AUX_MU_IER_REG, 0);
  iowrite32(AUX_MU_CNTL_REG, 0);
  iowrite32(AUX_MU_LCR_REG, 3);
  iowrite32(AUX_MU_MCR_REG, 0);
  iowrite32(AUX_MU_IER_REG, 0);
  iowrite32(AUX_MU_IIR_REG, 0xc6);
  iowrite32(AUX_MU_BAUD_REG, 270);

  bcm283x_gpio_setup_for_miniuart();

  iowrite32(AUX_MU_CNTL_REG, 3);
}

#if 0
uint8_t getc(void)
{
  while (!(ioread32(AUX_MU_LSR_REG) & 0x01));
  return ioread32(AUX_MU_IO_REG);
}


void putc(char c)
{
  if (c == '\n') {
    putc('\r');
  }

  while (!(ioread32(AUX_MU_LSR_REG) & 0x20));
  iowrite32(AUX_MU_IO_REG, c);
}


void puts(const char * str)
{
  while (*str) putc(*str++);
}

static const char hexDigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

void printHexLongLong(unsigned long long n)
{
  int i;

  for (i = 60; i >= 0; i -= 4) {
    putc(hexDigits[(n & (0xf << i)) >> i]);
  }
}

void printHexLong(unsigned int n)
{
  int i;

  for (i = 28; i >= 0; i -= 4) {
    putc(hexDigits[(n & (0xf << i)) >> i]);
  }
}

void printHexByte(unsigned int n)
{
  int i;

  for (i = 4; i >= 0; i -= 4) {
    putc(hexDigits[(n & (0xf << i)) >> i]);
  }
}

void printHexShort(unsigned int n)
{
  int i;

  for (i = 12; i >= 0; i -= 4) {
    putc(hexDigits[(n & (0xf << i)) >> i]);
  }
}
#endif

/*
 * Research v7 Interfaces
 */



#if 0
bcm283x_muopen
bcm283x_muclose
bcm283x_muread
bcm283x_muwrite
bcm283x_muxint
bcm283x_murint
bcm283x_muioctl
bcm283x_mustart
#endif


#if 0
/* TODO: fix this when we have the proper headers in place */
#if ! defined(MSGBUFS)
#define MSGBUFS 128  /* Characters saved from error messages */
char msgbuf[MSGBUFS]; /* saved "printf" characters */
#endif


static char *msgbufp = msgbuf;      /* Next saved printf character */

/*
 * Print a character on console.
 * Attempts to save and restore device
 * status.
 * If the switches are 0, all
 * printing is inhibited.
 *
 * Whether or not printing is inhibited,
 * the last MSGBUFS characters
 * are saved in msgbuf for inspection later.
 */
void putchar(unsigned int c)
{
#if 1
  putc(c);
#else
  uint32_t s;
  uint32_t timo;

#if 1
  if (c != '\0' && c != '\r' && c != 0177) {
    *msgbufp++ = c;

    if (msgbufp >= &msgbuf[MSGBUFS]) {
      msgbufp = msgbuf;
    }
  }
#endif

#if 0
  /*
   * If last char was a break or null, don't print - this is NOT how a modern UART works... will figure it out
   */
  if ((KLADDR->rbuf&0177) == 0) {
    return;
  }
#endif

  /*
   * Try waiting for the console tty to come ready,
   * otherwise give up after a reasonable time.
   */
  timo = 30000;

  while (!(ioread32(AUX_MU_LSR_REG) & 0x20)) {
    if (--timo == 0) {
      break;
    }
  }

  if (c == 0) {
    return;
  }

  /* s = ioread32(AUX_MU_IER_REG); */
  /* iowrite32(AUX_MU_IER_REG, 0); */

  if (c == '\n') {
    putchar('\r');
  }

  iowrite32(AUX_MU_IO_REG, c);

  timo = 30000;

  while (!(ioread32(AUX_MU_LSR_REG) & 0x20)) {
    if (--timo == 0) {
      break;
    }
  }

  /* iowrite32(AUX_MU_IER_REG, s); */
#endif
}
#endif
