/*
 * Master boot record and partition table structs are based on those found in
 * SDCard.c by Leon de Boer.  The original code can be fetched from:
 *  https://github.com/LdB-ECM/Raspberry-Pi/tree/master/SD_FAT32
 */
#include "sd_io.h"
#include "bcm283x_irq.h"
#include "../arm1176jzfs.h"
#include "../bcm283x_machdep.h"

#include "../../h/param.h"
#include "../../h/user.h"
#include "../../h/prf.h"

struct volume_t {
  u32 present;
  u32 start;
  u32 count;
};

#define MAX_VOLUMES 4
static struct volume_t volumes[MAX_VOLUMES] = {
  { 0, 0, 0, },
  { 0, 0, 0, },
  { 0, 0, 0, },
  { 0, 0, 0, },
};

struct __attribute__((packed)) partinfo_t {
  u8 status;  /* active flag              */
  u8 sh;      /* start head               */
  u16 scs;    /* start cylinder/sector    */
  u8 type;    /* partition type           */
  u8 eh;      /* end head                 */
  u16 ecs;    /* end cylinder/sector      */
  u32 sect;   /* start sector (think LBA) */
  u32 nsect;  /* size in sectors (blocks) */
};

struct __attribute__((packed)) mbr_t {
  u8 filler[446];                  /* bootblock */
  struct partinfo_t partitions[4]; /* partition records (16x4) */
  u16 signature;                   /* 0xaa55 */
};

static uptr_t handle;
void sdxintr(void);

static void sdx_irq(void *arg)
{
  sdxintr();
}

int sdx_init(u32 hz)
{
  u8 buffer[512] __attribute__((aligned(4)));
  struct mbr_t* mbr;
  struct partinfo_t* pd;
  u32 i;
  u32 nvol;
  int ret;

  if (!(handle = sd_io.init((uptr_t)_bcm283x_iobase, hz)))
    return -ENODEV;
  if (0 != (ret = sd_io.iostart(handle, 0, 0, 0, (u8*)buffer, 512)))
    return ret;
  if (0 != (ret = sd_io.iowait(handle, 0, 1000000)))
    return ret;
  if (0 != (ret = sd_io.iofinish(handle, 0, (u8*)buffer, 512)))
    return ret;

  mbr = (struct mbr_t *)buffer;
  if (mbr->signature != 0xaa55)
    return -ENXIO;

  nvol = 0;

  for (i = 0; i < 4; ++i) {
    pd = &mbr->partitions[i];

    if (pd->type == 0x7f) {
      volumes[nvol].present = 1;
      volumes[nvol].start = pd->sect;
      volumes[nvol].count = pd->nsect;
      nvol++;
    }
  }

  if (!nvol)
    return -ENXIO;

  if (0 != (ret = bcm283x_register_irq_handler(GPU_IRQ_EMMC_INT,
      sdx_irq, (void *)handle)))
    return ret;

  return 0;
}


/* *****************************************************************************
 * *****************************************************************************
 * *** UNIX v7 Device Interface                                              ***
 * *****************************************************************************
 * ****************************************************************************/


/*
 * Raspberry Pi SD Card disk driver
 */

#include "../../h/param.h"
#include "../../h/systm.h"
#include "../../h/buf.h"
#include "../../h/dir.h"
#include "../../h/conf.h"
#include "../../h/user.h"

extern void iodone(struct buf *bp);
extern void physio(void (*strat)(struct buf *), struct buf *bp, int dev, int rw);
extern void deverror(struct buf *bp, int o1, int o2);

struct buf sdxtab;
struct buf rsdxbuf;

/* should not be here */
long	dk_wds[3];
long	dk_numb[3];

/*
 * Monitoring device number
 */
#define	DK_N	0

/*
 * Start I/O operations - runs at spl5 (set by the caller)
 */
void sdxstart(void)
{
  struct buf *bp;
  u32 ba;

  if (!(bp = sdxtab.b_actf))
    return;  /* nothing to do */

#if 0
  if (sdCard.type == SD_TYPE_UNKNOWN) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;  /* nothing to do: no card */
  }
#endif

  ba = bp->b_blkno + volumes[minor(bp->b_dev)].start;

  if (0 != sd_io.iostart(handle,
      (bp->b_flags & B_READ) ? 0 : 1, 1, ba, (u8*)bp->b_un.b_addr, 512)) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  sdxtab.b_active++;
  dk_busy |= (1 << DK_N);
  dk_numb[DK_N] += 1;
  dk_wds[DK_N] += (bp->b_bcount >> 6);  /* tracks clicks written/read to/from disk */
}


/*
 * Maintains the buffer ops list for the SD card, kicking off processing when
 * needed.
 */
void sdxstrategy(struct buf *bp)
{
  struct buf *dp;
  long sz;
  u32 unit;
  u32 s;

  sz = bp->b_bcount;
  sz = (sz + 511) >> 9;  /* round up to a number of blocks */

  unit = minor(bp->b_dev);

  if (unit >= MAX_VOLUMES || !volumes[unit].present) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;  /* nothing to do: bad unit (partition) */
  }

  if (bp->b_blkno + sz >= volumes[unit].count) {
    bp->b_flags |= B_ERROR;
    iodone(bp);
    return;
  }

  bp->av_forw = NULL;
  s = spl5();

  dp = &sdxtab;

  if (dp->b_actf)
    dp->b_actl->av_forw = bp;
  else
    dp->b_actf = bp;

  dp->b_actl = bp;

  if (!dp->b_active) {
    DSB;
    sdxstart();
    DSB;
  }

  splx(s);
}


void sdxintr(void)
{
  struct buf *bp;

  if (!sdxtab.b_active)
    return;

  bp = sdxtab.b_actf;

  if (0 != sd_io.iofinish(handle,
      (bp->b_flags & B_READ) ? 0 : 1, (u8*)bp->b_un.b_addr, 512))
    bp->b_flags |= B_ERROR;

  dk_busy &= ~(1<<DK_N);
  bp = sdxtab.b_actf;
  sdxtab.b_active = 0;

#if 0
  /* XXX: check for errors */
  {
     // deverror(bp, RPADDR->rper, RPADDR->rpds);
     // clear the error, reset the device etc. etc.
     // while error count is not too high, retry (see the rp driver and rk driver)
     // finally, bp->b_flags |= B_ERROR;
  }
#endif

  sdxtab.b_errcnt = 0;
  sdxtab.b_actf = bp->av_forw;
  bp->b_resid = 0;
  iodone(bp);
  sdxstart();
}


/*
 * Raw device read (character interface)
 */
void sdxread(int dev)
{
  physio(sdxstrategy, &rsdxbuf, dev, B_READ);
}


/*
 * Raw device write (character interface)
 */
void sdxwrite(int dev)
{
  physio(sdxstrategy, &rsdxbuf, dev, B_WRITE);
}
