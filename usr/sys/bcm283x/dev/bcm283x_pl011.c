#include "bcm283x_pl011.h"
#include "bcm283x_io.h"
#include "bcm283x_irq.h"
#include "../kstddef.h"
#include "../arm1176jzfs.h"
#include "../../h/types.h"
#include "../bcm283x_machdep.h"

#include "../../h/param.h"
#include "../../h/systm.h"
#include "../../h/dir.h"
#include "../../h/user.h"
#include "../../h/tty.h"
#include "../../h/prf.h"

#define PL011_OFFSET                  0x00201000
#define PL011_BASE                    ((_bcm283x_iobase) + (PL011_OFFSET))

#define PL011_DR_REG_OFFSET           0x00000000  /* Data Register                        */
#define PL011_RSRECR_REG_OFFSET       0x00000004  /* Receive status and error clear reg   */
#define PL011_FR_REG_OFFSET           0x00000018  /* Flag Register                        */
#define PL011_ILPR_REG_OFFSET         0x00000020  /* Disabled IrDA register               */
#define PL011_IBRD_REG_OFFSET         0x00000024  /* Integer BAUD Rate Divisor            */
#define PL011_FBRD_REG_OFFSET         0x00000028  /* Fractional BAUD Rate Divisor         */
#define PL011_LCRH_REG_OFFSET         0x0000002c  /* Line Control Register                */
#define PL011_CR_REG_OFFSET           0x00000030  /* Control Register                     */
#define PL011_IFLS_REG_OFFSET         0x00000034  /* Interrupt FIFO Level Select Register */
#define PL011_IMSC_REG_OFFSET         0x00000038  /* Interrupt Mask Set Clear Register    */
#define PL011_RIS_REG_OFFSET          0x0000003c  /* Raw Interrupt Status Register        */
#define PL011_MIS_REG_OFFSET          0x00000040  /* Masked Interrupt Status Register     */
#define PL011_ICR_REG_OFFSET          0x00000044  /* Interrupt Clear Register             */
#define PL011_DMACR_REG_OFFSET        0x00000048  /* DMA Control Register (disabled)      */
#define PL011_ITCR_REG_OFFSET         0x00000080  /* Test Control register                */
#define PL011_ITIP_REG_OFFSET         0x00000084  /* Integration test input reg           */
#define PL011_ITOP_REG_OFFSET         0x00000088  /* Integration test output reg          */
#define PL011_TDR_REG_OFFSET          0x0000008c  /* Test Data reg                        */

#define PL011_DR_REG                  ((PL011_BASE) + (PL011_DR_REG_OFFSET))
#define PL011_RSRECR_REG              ((PL011_BASE) + (PL011_RSRECR_REG_OFFSET))
#define PL011_FR_REG                  ((PL011_BASE) + (PL011_FR_REG_OFFSET))
#define PL011_ILPR_REG                ((PL011_BASE) + (PL011_ILPR_REG_OFFSET))
#define PL011_IBRD_REG                ((PL011_BASE) + (PL011_IBRD_REG_OFFSET))
#define PL011_FBRD_REG                ((PL011_BASE) + (PL011_FBRD_REG_OFFSET))
#define PL011_LCRH_REG                ((PL011_BASE) + (PL011_LCRH_REG_OFFSET))
#define PL011_CR_REG                  ((PL011_BASE) + (PL011_CR_REG_OFFSET))
#define PL011_IFLS_REG                ((PL011_BASE) + (PL011_IFLS_REG_OFFSET))
#define PL011_IMSC_REG                ((PL011_BASE) + (PL011_IMSC_REG_OFFSET))
#define PL011_RIS_REG                 ((PL011_BASE) + (PL011_RIS_REG_OFFSET))
#define PL011_MIS_REG                 ((PL011_BASE) + (PL011_MIS_REG_OFFSET))
#define PL011_ICR_REG                 ((PL011_BASE) + (PL011_ICR_REG_OFFSET))
#define PL011_DMACR_REG               ((PL011_BASE) + (PL011_DMACR_REG_OFFSET))
#define PL011_ITCR_REG                ((PL011_BASE) + (PL011_ITCR_REG_OFFSET))
#define PL011_ITIP_REG                ((PL011_BASE) + (PL011_ITIP_REG_OFFSET))
#define PL011_ITOP_REG                ((PL011_BASE) + (PL011_ITOP_REG_OFFSET))
#define PL011_TDR_REG                 ((PL011_BASE) + (PL011_TDR_REG_OFFSET))

#define PL011_FR_TXFE_BIT             7
#define PL011_FR_RXFF_BIT             6
#define PL011_FR_TXFF_BIT             5
#define PL011_FR_RXFE_BIT             4
#define PL011_FR_BUSY_BIT             3
#define PL011_FR_CTS_BIT              0

#define PL011_FR_TXFE                 BIT(PL011_FR_TXFE_BIT)
#define PL011_FR_RXFF                 BIT(PL011_FR_RXFF_BIT)
#define PL011_FR_TXFF                 BIT(PL011_FR_TXFF_BIT)
#define PL011_FR_RXFE                 BIT(PL011_FR_RXFE_BIT)
#define PL011_FR_BUSY                 BIT(PL011_FR_BUSY_BIT)
#define PL011_FR_CTS                  BIT(PL011_FR_CTS_BIT)

#define PL011_CR_CTSEN_BIT            15
#define PL011_CR_RTSEN_BIT            14
#define PL011_CR_RTS_BIT              11
#define PL011_CR_RXE_BIT              9
#define PL011_CR_TXE_BIT              8
#define PL011_CR_LBE_BIT              7
#define PL011_CR_UARTEN_BIT           0

#define PL011_CR_CTSEN                BIT(PL011_CR_CTSEN_BIT)
#define PL011_CR_RTSEN                BIT(PL011_CR_RTSEN_BIT)
#define PL011_CR_RTS                  BIT(PL011_CR_RTS_BIT)
#define PL011_CR_RXE                  BIT(PL011_CR_RXE_BIT)
#define PL011_CR_TXE                  BIT(PL011_CR_TXE_BIT)
#define PL011_CR_LBE                  BIT(PL011_CR_LBE_BIT)
#define PL011_CR_UARTEN               BIT(PL011_CR_UARTEN_BIT)

#define PL011_LCRH_SPS_BIT            7
#define PL011_LCRH_WLEN_SHIFT         5
#define PL011_LCRH_WLEN_MASK          (0x3 << (PL011_LCRH_WLEN_SHIFT))
#define PL011_LCRH_FEN_BIT            4
#define PL011_LCRH_STP2_BIT           3
#define PL011_LCRH_EPS_BIT            2
#define PL011_LCRH_PEN_BIT            1
#define PL011_LCRH_BRK_BIT            0

#define PL011_LCRH_SPS                BIT(PL011_LCRH_SPS_BIT)
#define PL011_LCRH_WLEN_8             (0x03 << (PL011_LCRH_WLEN_SHIFT))
#define PL011_LCRH_WLEN_7             (0x02 << (PL011_LCRH_WLEN_SHIFT))
#define PL011_LCRH_WLEN_6             (0x01 << (PL011_LCRH_WLEN_SHIFT))
#define PL011_LCRH_WLEN_5             (0x00 << (PL011_LCRH_WLEN_SHIFT))
#define PL011_LCRH_FEN                BIT(PL011_LCRH_FEN_BIT)
#define PL011_LCRH_STP2               BIT(PL011_LCRH_STP2_BIT)
#define PL011_LCRH_EPS                BIT(PL011_LCRH_EPS_BIT)
#define PL011_LCRH_PEN                BIT(PL011_LCRH_PEN_BIT)
#define PL011_LCRH_BRK                BIT(PL011_LCRH_BRK_BIT)

#define PL011_IFLS_TXIFLSEL_SHIFT     0
#define PL011_IFLS_TXIFLSEL_MASK      (0x7 << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_SHIFT     3
#define PL011_IFLS_RXIFLSEL_MASK      (0x7 << (PL011_IFLS_RXIFLSEL_SHIFT))

#define _IFLSEL_1_8_VAL               0
#define _IFLSEL_1_4_VAL               1
#define _IFLSEL_1_2_VAL               2
#define _IFLSEL_3_4_VAL               3
#define _IFLSEL_7_8_VAL               4
#define PL011_IFLS_TXIFLSEL_1_8       ((_IFLSEL_1_8_VAL) << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_TXIFLSEL_1_4       ((_IFLSEL_1_4_VAL) << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_TXIFLSEL_1_2       ((_IFLSEL_1_2_VAL) << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_TXIFLSEL_3_4       ((_IFLSEL_3_4_VAL) << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_TXIFLSEL_7_8       ((_IFLSEL_7_8_VAL) << (PL011_IFLS_TXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_1_8       ((_IFLSEL_1_8_VAL) << (PL011_IFLS_RXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_1_4       ((_IFLSEL_1_4_VAL) << (PL011_IFLS_RXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_1_2       ((_IFLSEL_1_2_VAL) << (PL011_IFLS_RXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_3_4       ((_IFLSEL_3_4_VAL) << (PL011_IFLS_RXIFLSEL_SHIFT))
#define PL011_IFLS_RXIFLSEL_7_8       ((_IFLSEL_7_8_VAL) << (PL011_IFLS_RXIFLSEL_SHIFT))

#define PL011_IMSC_OEIM_BIT           10
#define PL011_IMSC_BEIM_BIT           9
#define PL011_IMSC_PEIM_BIT           8
#define PL011_IMSC_FEIM_BIT           7
#define PL011_IMSC_RTIM_BIT           6
#define PL011_IMSC_TXIM_BIT           5
#define PL011_IMSC_RXIM_BIT           4
#define PL011_IMSC_CTSMIM_BIT         1

#define PL011_IMSC_OEIM               BIT(PL011_IMSC_OEIM_BIT)
#define PL011_IMSC_BEIM               BIT(PL011_IMSC_BEIM_BIT)
#define PL011_IMSC_PEIM               BIT(PL011_IMSC_PEIM_BIT)
#define PL011_IMSC_FEIM               BIT(PL011_IMSC_FEIM_BIT)
#define PL011_IMSC_RTIM               BIT(PL011_IMSC_RTIM_BIT)
#define PL011_IMSC_TXIM               BIT(PL011_IMSC_TXIM_BIT)
#define PL011_IMSC_RXIM               BIT(PL011_IMSC_RXIM_BIT)
#define PL011_IMSC_CTSMIM             BIT(PL011_IMSC_CTSMIM_BIT)

#define PL011_MIS_OEIM                (PL011_IMSC_OEIM)
#define PL011_MIS_BEIM                (PL011_IMSC_BEIM)
#define PL011_MIS_PEIM                (PL011_IMSC_PEIM)
#define PL011_MIS_FEIM                (PL011_IMSC_FEIM)
#define PL011_MIS_RTIM                (PL011_IMSC_RTIM)
#define PL011_MIS_TXIM                (PL011_IMSC_TXIM)
#define PL011_MIS_RXIM                (PL011_IMSC_RXIM)
#define PL011_MIS_CTSMIM              (PL011_IMSC_CTSMIM)

#define PL011_MIS_ERR                 ((PL011_MIS_OEIM)|(PL011_MIS_BEIM)|(PL011_MIS_PEIM)|(PL011_MIS_FEIM))

static unsigned int bcm283x_pl011_irq_registered;


/*
 * 3000000 has been set in machdep as the source clock frequency
 *
 * (3000000 / (16 * 115200) = 1.627
 * (0.627 * 64) + 0.5 = 40
 * Integral: 1, Fractional: 40
 */
#define PL011_115200_IBRD 1
#define PL011_115200_FBRD 40


static void bcm283x_pl011_reinit(u32 ibrd, u32 fbrd)
{
  int cr;
  /* mask all interrupts */
  iowrite32(PL011_IMSC_REG, 0);
  /* disable the UART */
  iowrite32(PL011_CR_REG, ioread32(PL011_CR_REG) & ~(PL011_CR_UARTEN));
  /* drain the receive buffer */
  while (!(ioread32(PL011_FR_REG) & PL011_FR_RXFE))
    (void)ioread32(PL011_DR_REG);
  /* wait for the transmit buffer to drain */
  while (!(ioread32(PL011_FR_REG) & PL011_FR_TXFE)) ;
  /* wait for the last character to leave the shift register */
  while ((ioread32(PL011_FR_REG) & PL011_FR_BUSY)) ;
  /* disable FIFOs to flush the UART */
  iowrite32(PL011_LCRH_REG, ioread32(PL011_LCRH_REG) & ~(PL011_LCRH_FEN));

  /* ack any outstanding interrupts */
  iowrite32(PL011_ICR_REG, 0x7FF);
  /* integer baud rate divisor */
  iowrite32(PL011_IBRD_REG, ibrd);
  /* fractional baud rate divisor */
  iowrite32(PL011_FBRD_REG, fbrd);
  /* 8bit words, FIFOs enabled */
  iowrite32(PL011_LCRH_REG, PL011_LCRH_WLEN_8|PL011_LCRH_FEN);
  /* rx fifo fires on 7/8 full, tx on 1/8 full */
  iowrite32(PL011_IFLS_REG, (PL011_IFLS_TXIFLSEL_1_8)|(PL011_IFLS_RXIFLSEL_7_8));
  /* rx & tx enable */
  cr = PL011_CR_TXE;
  iowrite32(PL011_CR_REG, cr);
  /* enable the UART */
  iowrite32(PL011_CR_REG, cr|PL011_CR_UARTEN);
}


void bcm283x_uart_early_init(void)
{
  DMB;
  bcm283x_pl011_reinit(PL011_115200_IBRD, PL011_115200_FBRD);
  DMB;
}


/*
 * Research v7 Interfaces
 */

char	msgbuf[MSGBUFS];	/* saved "printf" characters */
static char *msgbufp = msgbuf;      /* Next saved printf character */

void __uart_prepare_putchar(u32 *imsc)
{
  DMB;
  *imsc = ioread32(PL011_IMSC_REG);

  iowrite32(PL011_IMSC_REG, 0);
  while (!(ioread32(PL011_FR_REG) & PL011_FR_TXFE)) ;
  while ((ioread32(PL011_FR_REG) & PL011_FR_BUSY)) ;
}


void __uart_restore_putchar(u32 imsc)
{
  while (!(ioread32(PL011_FR_REG) & PL011_FR_TXFE)) ;
  while ((ioread32(PL011_FR_REG) & PL011_FR_BUSY)) ;
  DMB;
  iowrite32(PL011_IMSC_REG, imsc);
}


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
  if (c != '\0' && c != '\r' && c != 0177) {
    *msgbufp++ = c;

    if (msgbufp >= &msgbuf[MSGBUFS]) {
      msgbufp = msgbuf;
    }
  }

#if 0
  /*
   * If last char was a break or null, don't print - this is NOT how a modern UART works... will figure it out
   */
  if ((KLADDR->rbuf&0177) == 0) {
    return;
  }
#endif

  if (c == 0) {
    return;
  }

  if (c == '\n') {
    putchar('\r');
  }

  while (ioread32(PL011_FR_REG) & (PL011_FR_TXFF)) ;
  iowrite32(PL011_DR_REG, c);
}


/*
 * Character device interfaces
 */

/*
 * TODO:
 *  UART_LCRH can send BRK, character read (or IRQ) can receive BRK
 *  Parity, stopbits etc.
 */

#define BCM283X_PL011_DELAY 0
#define NPL011 1

struct tty pl011[NPL011];
int npl011 = NPL011;

extern int getc(struct clist *p);
typedef void (*timeout_cb_t)(void *);
extern void timeout(void (*fun)(void *), void *arg, int tim);
extern void ttrstrt(struct tty *tp);
extern void ttychars(struct tty *tp);
extern void ttyopen(dev_t dev, struct tty *tp);
extern void ttyclose(struct tty *tp);
extern void ttstart(struct tty *tp);
extern int ttread(struct tty *tp);
extern caddr_t ttwrite(struct tty *tp);
extern void ttyinput(int c, struct tty *tp);
extern int ttioccomm(int com, struct tty *tp, caddr_t addr, dev_t dev);
extern void wakeup(caddr_t chan);

extern char partab[];

typedef int (*t_oproc_t)();

static void bcm283x_pl011out(struct tty *tp);
static void bcm283x_pl011_irq(void *arg);

/* called on first teletype open */
void bcm283x_pl011open(dev_t dev, int flag)
{
  struct tty *tp;
  u32 cr;

  if (minor(dev)) {
    u.u_error = ENXIO;
    return;
  }

  tp = &pl011[minor(dev)];
  tp->t_addr = (caddr_t)PL011_BASE;
  tp->t_state |= WOPEN;
  tp->t_oproc = (t_oproc_t)bcm283x_pl011out;

  if ((tp->t_state & ISOPEN) == 0) {
    DMB;
    cr = ioread32(PL011_CR_REG);
    if (!(cr & PL011_CR_UARTEN)) {
      bcm283x_pl011_reinit(PL011_115200_IBRD, PL011_115200_FBRD);
      cr = ioread32(PL011_CR_REG);
      DMB;
    }
    if (!(cr & PL011_CR_RXE)) {
      iowrite32(PL011_CR_REG, cr|PL011_CR_RXE);
    }

    /* enable, set default speeds etc. */
    tp->t_state = ISOPEN|CARR_ON;
    tp->t_flags = ECHO|CRMOD;
    ttychars(tp);

    if (!bcm283x_pl011_irq_registered) {
      if (bcm283x_register_irq_handler(GPU_IRQ_UART_INT, bcm283x_pl011_irq, (void *)((u32)dev)) != 0) {
        panic("bcm283x_uart_early_init: failed to register IRQ handler");
      }

      bcm283x_pl011_irq_registered = 1;
    }

    iowrite32(PL011_IMSC_REG,
      PL011_IMSC_OEIM |    /* Overrun error */
      PL011_IMSC_BEIM |    /* Break error   */
      PL011_IMSC_PEIM |    /* Parity error  */
      PL011_IMSC_FEIM |    /* Framing error */
      PL011_IMSC_RTIM |    /* RX timeout    */
      /* PL011_IMSC_TXIM | */   /* Transmit IRQ  */
      PL011_IMSC_RXIM |    /* Receive IRQ   */
      PL011_IMSC_CTSMIM);  /* CTS IRQ - perhaps unnecessary? */
  }

  ttyopen(dev, tp);
}


/* called on last close */
void bcm283x_pl011close(dev_t dev)
{
  int cr;
  int lcrh;
  struct tty *tp;

  tp = &pl011[minor(dev)];
  ttyclose(tp);  /* flushes output queues */

  /* mask all interrupts */
  iowrite32(PL011_IMSC_REG, 0);
  DMB;
  cr = ioread32(PL011_CR_REG);
  /* disable the receiver */
  cr &= ~(PL011_CR_RXE);
  iowrite32(PL011_CR_REG, cr);
  /* drain the receive buffer */
  while (!(ioread32(PL011_FR_REG) & PL011_FR_RXFE))
    (void)ioread32(PL011_DR_REG);
  /* wait for the transmit buffer to drain */
  while (!(ioread32(PL011_FR_REG) & PL011_FR_TXFE)) ;
  /* wait for the last character to leave the shift register */
  while ((ioread32(PL011_FR_REG) & PL011_FR_BUSY)) ;
  /* disable FIFOs to flush the UART */
  lcrh = ioread32(PL011_LCRH_REG);
  iowrite32(PL011_LCRH_REG, lcrh & ~(PL011_LCRH_FEN));
  /* ack any outstanding interrupts */
  iowrite32(PL011_ICR_REG, 0x7FF);
  /* re-enable the FIFOs */
  iowrite32(PL011_LCRH_REG, lcrh);
  DMB;
}


void bcm283x_pl011read(dev_t dev)
{
  struct tty *tp;

  tp = &pl011[minor(dev)];
  ttread(tp);
}


void bcm283x_pl011write(dev_t dev)
{
  struct tty *tp;

  tp = &pl011[minor(dev)];
  ttwrite(tp);
}


static void bcm283x_pl011start(struct tty *tp, u32 *moar)
{
  s32 c;
  u32 sts;

  while (!(sts = (ioread32(PL011_FR_REG) & PL011_FR_TXFF))) {
    if ((c = getc(&tp->t_outq)) >= 0) {
      if (tp->t_flags & RAW) {
        iowrite32(PL011_DR_REG, c);
      } else if (c <= 0177) {
        iowrite32(PL011_DR_REG, c);
      } else {
        timeout((timeout_cb_t)ttrstrt, tp, (c & 0177) + BCM283X_PL011_DELAY);
        tp->t_state |= TIMEOUT;
        break;
      }
    } else {
      *moar = 1;
      break;
    }
  }

  /*
   * If the queue ran out of characters or the output FIFO blocked we might
   * have more work to do.
   */
  if (!*moar && sts) {
    *moar = 1;
  }
}


static void bcm283x_pl011out(struct tty *tp)
{
  u32 moar;

  moar = 0;
  DMB;
  bcm283x_pl011start(tp, &moar);

  /*
   * There might be more work to do after the output function is called.  If
   * there is, ensure that TX IRQs are enabled so that the queue can be
   * pumped out by the IRQ mechanism.
   */
  if (moar) {
    iosetbits32(PL011_IMSC_REG, PL011_IMSC_TXIM);
  }

  DMB;
}


static void bcm283x_pl011xint(dev_t dev)
{
  struct tty *tp;
  u32 moar;
  int s;

  tp = &pl011[minor(dev)];

  s = spl5();

  if (tp->t_outq.c_cc) {
    moar = 0;
    bcm283x_pl011start(tp, &moar);

    if (moar) {
      iosetbits32(PL011_IMSC_REG, PL011_IMSC_TXIM);
    }
  } else {
    /*
     * When our TX IRQ fires and we have no data in the output queue, we have
     * successfully pumped the queue dry, sending all characters to the UART.
     */
    ioclrbits32(PL011_IMSC_REG, PL011_IMSC_TXIM);
  }

  splx(s);

  if (tp->t_state & ASLEEP && tp->t_outq.c_cc <= TTLOWAT) {
    tp->t_state &= ~ASLEEP;
    wakeup((caddr_t)&tp->t_outq);
  }
}


static void bcm283x_pl011rint(dev_t dev)
{
  u32 c;
  struct tty *tp;

  tp = &pl011[minor(dev)];

  if (!(ioread32(PL011_FR_REG) & PL011_FR_RXFE)) {
    c = ioread32(PL011_DR_REG);
    ttyinput(c & 0xff, tp);  /* TODO: check for BRK here */
  }
}


static void bcm283x_pl011_irq(void *arg)
{
  u32 mis;
  dev_t dev;

  dev = (dev_t)((u32)arg);
  mis = ioread32(PL011_MIS_REG);

  if (mis & PL011_MIS_ERR) {
    printf("bcm283x_pl011_irq: PL011_MIS_REG: 0x%x\n", mis);
    panic("bcm283x_pl011_irq: got an error");
  }

  if (mis & PL011_MIS_RXIM || mis & PL011_MIS_RTIM) {
    bcm283x_pl011rint(dev);
  }

  if (mis & PL011_MIS_TXIM) {
    bcm283x_pl011xint(dev);
  }

  /*
   * TODO: we enable a lot of interrupts, we should handle
   * them or not enable them.
   */

  iowrite32(PL011_ICR_REG, mis);
}


/*
 * ioctl veneer
 *
 * Derives the tty struct and calls into common ioctl code.
 */
void bcm283x_pl011ioctl(dev_t dev, int cmd, caddr_t addr, int flag)
{
  if (ttioccomm(cmd, &pl011[minor(dev)], addr, dev) == 0) {
    u.u_error = ENOTTY;
  }
}


/*
 * Stop output on a line
 *
 * Not convinced we need this...
 */
void bcm283x_pl011stop(dev_t dev)
{
  /* struct tty *tp; */
  int s;

  /* tp = &pl011[minor(dev)]; */

  s = spl6();
  DMB;
  ioclrbits32(PL011_IMSC_REG, PL011_IMSC_TXIM);
  DMB;
  splx(s);
}
