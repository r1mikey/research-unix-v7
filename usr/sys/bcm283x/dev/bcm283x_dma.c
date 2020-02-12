#include "bcm283x_dma.h"

#include "../arm1176jzfs.h"
#include "bcm283x_io.h"
/* #include "irq.h" */

#define CB_ALIGN_WORDS                  8
#define CB_ALIGN_BYTES                  (4 * CB_ALIGN_WORDS)


/*
 * 8 * 32 bits = 256 bits, 32 bytes must be aligned to a 256 bits
 */
typedef struct dma_ctrl_block_t {
  uint32_t ti;           /* transfer information       */
  uint32_t source_ad;    /* source address             */
  uint32_t dest_ad;      /* destination address        */
  uint32_t txfr_len;     /* transfer length            */
  uint32_t stride;       /* 2d mode stride             */
  uint32_t nextconnbk;   /* next control block address */
  uint32_t reserved[2];  /* reserved - set to zero     */
} __attribute__((aligned(CB_ALIGN_BYTES))) dma_ctrl_block_t;


/* 4 * 32 bits + 2 * 16 bits => 160 bits, 20 bytes */
typedef struct dma_channel_t {
  uint32_t chan_num;     /* DMA channel number                              */
  dma_ctrl_block_t *cb;  /* control block sent to the DMA channel           */
  void *flush;           /* buffer to flush after DMA from device to memory */
  size_t len;            /* length of the 'flush' buffer                    */
  uint16_t wchan;        /* wait channel for scheduling sleep and wakeup    */
  uint16_t done;         /* DMA complete indicator                          */
} dma_channel_t;


#define DMAC_OFFSET                     0x00007000
#define DMAC_BASE                       ((_bcm283x_iobase) + DMAC_OFFSET)

#define NUM_DMAC_CHANNELS               7  /* see INT_ENABLE_ALL_MASK */
#define CHANNEL_SIZE                    0x00000100

#define CHAN_CS_REG                     0x00000000
#define CHAN_CS_RESET                   0x80000000
#define CHAN_CS_CLEAR_CONFIG            0x00000000
#define CHAN_CS_CLEARDOWN               0x00000006
#define CHAN_CS_ERROR                   0x00000100
#define CHAN_CS_INT                     0x00000004
#define CHAN_CS_END                     0x00000002
#define CHAN_CS_ACTIVE                  0x00000001

#define CHAN_DEBUG_REG                  0x00000020

#define CHAN_DEBUG_CLEARDOWN            0x00000007

#define CHAN_CONBLK_AD_REG              0x00000004

#define CHAN_IO_BASE_N(N)               (DMAC_BASE + (N * CHANNEL_SIZE))
#define CHAN_REG(N, REGOFF)             (CHAN_IO_BASE_N(N) + REGOFF)

/* global registers */
#define INT_STATUS_REG                  0x00000fe0
#define INT_ENABLE_REG                  0x00000ff0

#define INT_ENABLE_ALL_MASK             0x0000007f  /* see NUM_DMAC_CHANNELS */

#ifndef BLOCKALIGN
#define BLOCKALIGN                      64
#endif


extern devaddr_t _bcm283x_iobase;          /* peripheral base address */

dma_channel_t dmac_channels[NUM_DMAC_CHANNELS];
dma_ctrl_block_t dmac_control_blocks[NUM_DMAC_CHANNELS] __attribute__((aligned(CB_ALIGN_BYTES)));


static void dmaflush(int clean, void *p, uint32_t len)
{
#if 0
  uint32_t s = (uint32_t)p;
  uint32_t e = (uint32_t)p + len;

  if (clean) {
    s &= ~(BLOCKALIGN - 1);
    e += (BLOCKALIGN - 1);
    e &= ~(BLOCKALIGN - 1);
    cachedwbse((void*)s, e - s);
    return;
  }

  if (s & (BLOCKALIGN - 1)) {
    s &= ~(BLOCKALIGN - 1);
    cachedwbinvse((void*)s, BLOCKALIGN);
    s += BLOCKALIGN;
  }

  if (e & (BLOCKALIGN - 1)) {
    e &= ~(BLOCKALIGN - 1);

    if (e < s) {
      return;
    }

    cachedwbinvse((void*)e, BLOCKALIGN);
  }

  if (s < e) {
    cachedinvse((void *)s, e - s);
  }
#endif
}


#if 0
static void chan_intr(void *arg)
{
  dma_channel_t *chan;

  chan = arg;
  iowrite32(CHAN_REG(chan->chan_num, CHAN_CS_REG), CHAN_CS_INT);
  chan->done = 1;
  /* wakeup(&chan->wchan); */
}
#endif


int dmawait(uint32_t c)
{
  dma_channel_t *chan;
  uint32_t cs;
  /* int s; */

  chan = &dmac_channels[c];

  /* s = spl6(); */
  while (!chan->done) {
    /* sleep(&chan->wchan, PRIBIO); */

    if (chan->done) {
      break;
    }
  }
  /* splx(s); */

  chan->done = 0;

  if (chan->flush && chan->len) {
    dmaflush(0, chan->flush, chan->len);
    chan->flush = NULL;
    chan->len = 0;
  }

  cs = ioread32(CHAN_REG(c, CHAN_CS_REG));

  if ((cs & (CHAN_CS_ERROR|CHAN_CS_END|CHAN_CS_ACTIVE)) != CHAN_CS_END) {
    iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_RESET);
    iowrite32(CHAN_REG(c, CHAN_DEBUG_REG), CHAN_DEBUG_CLEARDOWN);
    /* return -EIO; */  /* TODO: iff CHAN_CS_ACTIVE, then it's a timeout */
    return -1;
  }

  iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_INT|CHAN_CS_END);
  return 0;
}


int dmastart(uint32_t c, uint32_t drqdev, uint32_t dmadir,
    void *src, void *dst, size_t len)
{
  dma_channel_t *chan;
  dma_ctrl_block_t *cb;
  /* uint32_t ti; */

  if (c >= NUM_DMAC_CHANNELS) {
    /* return -EINVAL; */
    return -1;
  }

  if (drqdev == DMAC_DRQ_DEVICE_UNUSED || drqdev > DMAC_DRQ_DEVICE_MAX) {
    /* return -EINVAL; */
    return -1;
  }

  chan = &dmac_channels[c];
  cb = chan->cb;

  /* ti = 0; */

  switch (dmadir) {
    case DMA_DIR_DEVICE_TO_MEMORY:
      chan->flush = dst;
      dmaflush(1, dst, len);
      /* ti = Srcdreq | Destinc; */
      /* cb->source_ad = dmaioaddr(src); */
      /* cb->dest_ad = dmaaddr(dst); */
      break;
    case DMA_DIR_MEMORY_TO_DEVICE:
      chan->flush = NULL;
      dmaflush(1, src, len);
      /* ti = Destdreq | Srcinc; */
      /* cb->source_ad = dmaaddr(src); */
      /* cb->dest_ad = dmaioaddr(dst); */
      break;
    case DMA_DIR_MEMORY_TO_MEMORY:
      chan->flush = dst;
      dmaflush(1, dst, len);
      dmaflush(1, src, len);
      /* ti = Srcinc | Destinc; */
      /* cb->source_ad = dmaaddr(src); */
      /* cb->dest_ad = dmaaddr(dst); */
      break;
    default:
      /* return -EINVAL; */
      return -1;
  }

  chan->len = len;

  /* cb->ti = ti | drqdev<<Permapshift | Inten; */
  cb->txfr_len = len;
  cb->stride = 0;
  cb->nextconnbk = 0;
  dmaflush(1, cb, sizeof(*cb));

  iowrite32(CHAN_REG(c, CHAN_DEBUG_REG), CHAN_DEBUG_CLEARDOWN);
  iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_CLEAR_CONFIG);
  /* udelay(1); */
  /* iowrite32(CHAN_REG(c, CHAN_CONBLK_AD_REG), dmaaddr(cb)); */

  iowrite32(DMAC_BASE + INT_STATUS_REG, 0x00000000); /* feels bad, technically wrong?! */
  iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_INT | CHAN_CS_END);
  /* udelay(1); */
  DSB;
  ISB;

  iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_ACTIVE);
  return 0;
}


int bcm283x_dma_init(void)
{
  /* int rc; */
  uint32_t c;
  dma_channel_t *chan;

  iosetbits32(DMAC_BASE + INT_ENABLE_REG, 0x00000000);

  for (c = 0; c < NUM_DMAC_CHANNELS; ++c) {
    iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_RESET);
  }

  for (c = 0; c < NUM_DMAC_CHANNELS; ++c) {
    chan = &dmac_channels[c];
    chan->chan_num = c;
    chan->cb = &dmac_control_blocks[c];
    chan->flush = NULL;
    chan->len = 0;
    chan->wchan = 0;
    chan->done = 0;
  }

  for (c = 0; c < NUM_DMAC_CHANNELS; ++c) {
    chan = &dmac_channels[c];

    while (iotstbits32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_RESET));
    /* rc = register_irq_handler(DMA_CHAN_IRQ(c), chan_intr, chan); */

    /* if (rc != 0) { */
      for (c = 0; c < NUM_DMAC_CHANNELS; ++c) {
        /* (void)deregister_irq_handler(DMA_CHAN_IRQ(c)); */
        iowrite32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_RESET);
        while (iotstbits32(CHAN_REG(c, CHAN_CS_REG), CHAN_CS_RESET));
      }

      iosetbits32(DMAC_BASE + INT_ENABLE_REG, 0x00000000);
      /* return rc; */
    /* } */
  }

  iosetbits32(DMAC_BASE + INT_ENABLE_REG, INT_ENABLE_ALL_MASK);
  return 0;
}
