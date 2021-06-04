/*
 * Arasan eMMC SD Host Controller interface, as found on the Raspberry Pi.
 */
#include "../../h/types.h"
#include "../../h/user.h"
#include "../../h/prf.h"
#include "arasan_sd_regs.h"
#include "bcm283x_io.h"
#include "bcm283x_irq.h"
#include "bcm283x_systimer.h"
#include "../arm1176jzfs.h"
#include "sd_io.h"
#include "sd_cmd.h"

#define FREQ_SETUP   400000    /* 400 Khz */
#define FREQ_NORMAL  25000000  /* 25 Mhz  */
#define FREQ_HS      50000000  /* 50 Mhz  */

struct arasan_sd_io_t {
  uptr_t base;
  u32 extclk;
  u32 sdver;
  int fast;
};

#define NUM_ARASAN_SDHCI 1
static struct arasan_sd_io_t arasan_sd_io[NUM_ARASAN_SDHCI];

#define ARASAN_REGADDR(__io, __offset)    ((caddr_t)((__io->base)+(EMMC_OFFSET)+(__offset)))
static u32 arasan_sd_readreg(struct arasan_sd_io_t *io, u32 a)
{
  return ioread32(ARASAN_REGADDR(io, a));
}

static void arasan_sd_writereg(struct arasan_sd_io_t *io, u32 a, u32 v)
{
  udelay(io->fast ? 2 : 20);
  iowrite32(ARASAN_REGADDR(io, a), v);
}
#undef ARASAN_REGADDR

static struct arasan_sd_io_t * arasan_sd_io_get(uptr_t iobase, u32 extclk)
{
  struct arasan_sd_io_t *io;
  int i;

  for (i = 0; i < NUM_ARASAN_SDHCI; ++i)
    if (arasan_sd_io[i].base == iobase)
      return &arasan_sd_io[i];

  for (i = 0; i < NUM_ARASAN_SDHCI; ++i) {
    if (arasan_sd_io[i].base == 0) {
      io = &arasan_sd_io[i];
      io->base = iobase;
      break;
    }
  }

  if (i >= NUM_ARASAN_SDHCI)
    return NULL;

  return io;
}

static void arasan_sd_io_put(struct arasan_sd_io_t *io)
{
  int i;

  for (i = 0; i < NUM_ARASAN_SDHCI; ++i) {
    if (io == &arasan_sd_io[i]) {
      io->base = 0;
      io->extclk = 0;
      io->sdver = 0;
      io->fast = 0;
      break;
    }
  }
}

static struct arasan_sd_io_t * from_uptr(uptr_t handle)
{
  int i;

  for (i = 0; i < NUM_ARASAN_SDHCI; ++i)
    if (handle == (uptr_t)(&arasan_sd_io[i]))
      return &arasan_sd_io[i];

  return NULL;
}

static uptr_t to_uptr(struct arasan_sd_io_t *io)
{
  return (uptr_t)io;
}

static int arasan_sd_await_mask_in_reg(struct arasan_sd_io_t *io, u32 reg, u32 mask, u32 *final, u32 max_us)
{
  u32 i;
  u32 v;

  for (i = 0; i < max_us; ++i) {
    if (!i)
      udelay(1);
    if ((v = arasan_sd_readreg(io, reg)) & mask)
      break;
    udelay(1);
  }

  if (final)
    *final = v;

  if (i >= max_us)
    return -EBUSY;

  return 0;
}

static int arasan_sd_await_mask_not_in_reg(struct arasan_sd_io_t *io, u32 reg, u32 mask, u32 *final, u32 max_us)
{
  u32 i;
  u32 v;

  for (i = 0; i < max_us; ++i) {
    if (!i)
      udelay(1);
    if (!((v = arasan_sd_readreg(io, reg)) & mask))
      break;
    udelay(1);
  }

  if (final)
    *final = v;

  if (i >= max_us)
    return -EBUSY;

  return 0;
}

static int arasan_sd_reset_circuit(struct arasan_sd_io_t *io, u32 mask)
{
  int ret;
  u32 v;

  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET, mask);
  if (0 != (ret = arasan_sd_await_mask_not_in_reg(io,
      EMMC_CONTROL1_REG_OFFSET, mask, &v, 100000)))
    return ret;

  return 0;
}

static int arasan_sd_reset(struct arasan_sd_io_t *io)
{
  int ret;

  arasan_sd_writereg(io, EMMC_CONTROL0_REG_OFFSET, 0U);
  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET, 0U);
  arasan_sd_writereg(io, EMMC_CONTROL2_REG_OFFSET, 0U);
  if (0 != (ret = arasan_sd_reset_circuit(io,
      SRST_DATA_MASK|SRST_CMD_MASK|SRST_HC_MASK)))
    return ret;
  arasan_sd_writereg(io, EMMC_CONTROL0_REG_OFFSET, 0U);
  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET, 0U);
  arasan_sd_writereg(io, EMMC_CONTROL2_REG_OFFSET, 0U);

  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET,
    (arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET) & ~(DATA_TOUNIT_MASK)) |
      (0xeU << DATA_TOUNIT_SHIFT));

  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET,
    (arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET) & ~(CLK_INTLEN_MASK)) |
      (0x1U << CLK_INTLEN_SHIFT));

  return 0;
}

/*
 * The Arasan IP is a version 3.00 host controller, uses the 1-bit divided clock.
 */
static u32 arasan_sd_get_clkdiv(struct arasan_sd_io_t *io, u32 freq)
{
  u32 denom;

  /* Version 3.0 divisors are multiples of two up to 1023 * 2 */
  /* From FreeBSD sys/dev/sdhci/sdhci.c */
  if (freq >= io->extclk)
    denom = 1;
  else {
    for (denom = 2; denom < 2046; denom += 2) {
      if ((io->extclk / denom) <= freq)
        break;
    }
  }

  return denom >> 1;
}

static int arasan_sd_setclk(struct arasan_sd_io_t *io, u32 freq)
{
  u32 cdiv;
  int ret;
  u32 v;

  cdiv = arasan_sd_get_clkdiv(io, freq);

  if (0 != (ret = arasan_sd_await_mask_not_in_reg(io, EMMC_STATUS_REG_OFFSET,
      (CMD_INHIBIT_MASK|DAT_INHIBIT_MASK), NULL, 1000000)))
    return ret;

  v = arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET);
  if (v & (CLK_EN_MASK)) {
    v &= ~(CLK_EN_MASK);
    arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET, v);
    udelay(10);
  }

  v &= ~(CLK_FREQ_MS2_MASK|CLK_FREQ8_MASK|CLK_STABLE_MASK);
  v |= ((cdiv & 0xff) << CLK_FREQ8_SHIFT);
  v |= (((cdiv & 0x300) >> 8) << CLK_FREQ_MS2_SHIFT);

  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET, v);
  udelay(10);

  arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET,
    (arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET) | CLK_EN_MASK));
  udelay(10);

  return arasan_sd_await_mask_in_reg(io,
    EMMC_CONTROL1_REG_OFFSET, CLK_STABLE_MASK, NULL, 1000);
}

static void arasan_sd_io_fini(uptr_t handle)
{
  struct arasan_sd_io_t *io;

  if (!(io = from_uptr(handle)))
    return;

  DMB;
  arasan_sd_reset(io);
  arasan_sd_writereg(io, EMMC_IRPT_EN_REG_OFFSET, 0x0U);
  arasan_sd_writereg(io, EMMC_IRPT_MASK_REG_OFFSET, 0x0U);
  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, 0xffffffffU);
  arasan_sd_io_put(io);
  DMB;
}

static uptr_t arasan_sd_io_init(uptr_t iobase, u32 extclk)
{
  struct arasan_sd_io_t *io;
  int ret;

  if (!(io = arasan_sd_io_get(iobase, extclk)))
    return 0;

  io->extclk = extclk;
  io->fast = 0;

  DMB;

  if (0 != (ret = arasan_sd_reset(io)))
    goto out;

  io->sdver =
    (arasan_sd_readreg(io, EMMC_SLOTISR_VER_REG_OFFSET) & SDVERSION_MASK)
      >> SDVERSION_SHIFT;

  if (0 != (ret = arasan_sd_setclk(io, FREQ_SETUP)))
    goto out;
  io->fast = 0;

  arasan_sd_writereg(io, EMMC_IRPT_EN_REG_OFFSET, 0U);
  arasan_sd_writereg(io, EMMC_IRPT_MASK_REG_OFFSET, 0xffffffffU);
  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, 0xffffffffU);

out:
  DMB;

  if (ret) {
    arasan_sd_io_put(io);
    return 0;
  }

  return to_uptr(io);
}

/*
 * Responses and the Index Check Enable and CRC Check Enable parts are from
 * PartA2_SD Host_Controller_Simplified_Specification_Ver4.20.pdf, page 60
 * (marked as 47).
 *
 * Block Count Enable and Block Count are from page 57 of the same document,
 * but are not as clearly stated.
 *
 * Additional information about the SDIO-specific responses comes from
 * PartE1_SDIO_Simplified_Specification_Ver3.00.pdf and, again, is not
 * as clear.
 */
static int arasan_sd_io_cmdtm_get(u32 cmd, u32 *c)
{
  if (!SD_CMD_IS_VALID(cmd) || !c)
    return -EINVAL;

  *c = ((SD_CMD_GET_CMD_INDEX(cmd)) << (CMD_INDEX_SHIFT));
  switch (SD_CMD_GET_RSP_TYPE(cmd)) {
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_NONE):
      *c |= (CMD_NO_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R1):
      *c |= (CMD_IXCHK_EN_MASK|CMD_CRCCHK_EN_MASK|CMD_48BIT_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R1B):  /* intentional fallthrough */
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R5B):
      *c |= (CMD_IXCHK_EN_MASK|CMD_CRCCHK_EN_MASK|CMD_BUSY48BIT_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R2):
      *c |= (CMD_CRCCHK_EN_MASK|CMD_136BIT_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R3):  /* intentional fallthrough */
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R4):
      *c |= (CMD_48BIT_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R5):  /* intentional fallthrough */
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R6):
      *c |= (CMD_IXCHK_EN_MASK|CMD_CRCCHK_EN_MASK|CMD_48BIT_RESP);
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R7):
      *c |= (CMD_IXCHK_EN_MASK|CMD_CRCCHK_EN_MASK|CMD_48BIT_RESP);
      break;
    default:
      return -EINVAL;
  }

  switch (cmd) {
    case SD_CMD_GEN_CMD:  /* XXX: don't know what to do here */
      *c |= (CMD_ISDATA_MASK);
      break;
    case SD_CMD_READ_SINGLE_BLOCK:    /* intentional fallthrough */
    case SD_CMD_SEND_TUNING_BLOCK:    /* intentional fallthrough */
    case SD_CMD_SEND_WRITE_PROT:      /* intentional fallthrough */
    case SD_CMD_READ_EXTR_SINGLE:     /* intentional fallthrough */
    case SD_ACMD_SEND_NUM_WR_BLOCKS:  /* intentional fallthrough */
    case SD_ACMD_SEND_SCR:
      *c |= (CMD_ISDATA_MASK|TM_DAT_DIR_CARD2HOST);
      break;
    case SD_CMD_READ_MULTIPLE_BLOCK:  /* intentional fallthrough */
    case SD_CMD_READ_EXTR_MULTI:
      *c |= (CMD_ISDATA_MASK|TM_DAT_DIR_CARD2HOST|
        TM_MULTI_BLOCK_MASK|TM_BLKCNT_EN_MASK);
      break;
    case SD_CMD_WRITE_BLOCK:          /* intentional fallthrough */
    case SD_CMD_PROGRAM_CSD:          /* intentional fallthrough */
    case SD_CMD_WRITE_EXTR_SINGLE:
      *c |= (CMD_ISDATA_MASK|TM_DAT_DIR_HOST2CARD);
      break;
    case SD_CMD_WRITE_MULTIPLE_BLOCK: /* intentional fallthrough */
    case SD_CMD_WRITE_EXTR_MULTI:
      *c |= (CMD_ISDATA_MASK|TM_DAT_DIR_HOST2CARD|
        TM_MULTI_BLOCK_MASK|TM_BLKCNT_EN_MASK);
      break;
    case SD_CMD_STOP_TRANSMISSION:
      *c |= (CMDTM_CMD_TYPE_ABORT);
      break;
    default:
      break;
  }

  return 0;
}

static int arasan_sd_io_cmd(uptr_t handle, u32 cmd, u32 arg, u32 *resp)
{
  struct arasan_sd_io_t *io;
  u32 c;
  int ret;
  u32 i;
  u32 v;

  if (!(io = from_uptr(handle)))
    return -EINVAL;

  if (0 != (ret = arasan_sd_io_cmdtm_get(cmd, &c)))
    return ret;

  DMB;
  if (0 != (ret = arasan_sd_await_mask_not_in_reg(io, EMMC_STATUS_REG_OFFSET,
      (CMD_INHIBIT_MASK|DAT_INHIBIT_MASK), NULL, 1000000))) {
    DMB;
    return ret;
  }

  /*
   * A "go to the idle state" is sent on card insertion, so reset data width
   * and clock speed when we see this command.
   */
  if (cmd == SD_CMD_GO_IDLE_STATE) {
    arasan_sd_writereg(io, EMMC_CONTROL0_REG_OFFSET,
      arasan_sd_readreg(io, EMMC_CONTROL0_REG_OFFSET) &
        ~(HCTL_DWIDTH_MASK|HCTL_HS_EN_MASK));
    if (0 != (ret = arasan_sd_setclk(io, FREQ_SETUP))) {
      DMB;
      return ret;
    }
    io->fast = 0;
  }

  if ((i = (arasan_sd_readreg(io, EMMC_INTERRUPT_REG_OFFSET) & ~CARD_MASK)))
    arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, i);
  arasan_sd_writereg(io, EMMC_IRPT_MASK_REG_OFFSET,
    arasan_sd_readreg(io, EMMC_IRPT_MASK_REG_OFFSET)|(CMD_DONE_MASK|ERR_MASK));

  arasan_sd_writereg(io, EMMC_ARG1_REG_OFFSET, arg);
  arasan_sd_writereg(io, EMMC_CMDTM_REG_OFFSET, c);
  if (cmd == SD_CMD_SEND_IF_COND || cmd == SD_CMD_APP_CMD)
    udelay(100);
  else if (cmd == SD_ACMD_SD_SEND_OP_COND)
    udelay(1000);

  ret = arasan_sd_await_mask_in_reg(io,
    EMMC_INTERRUPT_REG_OFFSET, CMD_DONE_MASK|ERR_MASK, &i, 1000000);  /* TODO: way too long */
  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET,
    i & ~(DATA_DONE_MASK|WRITE_RDY_MASK|READ_RDY_MASK|CARD_MASK));
  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, 0xffff0001); /* XXX: yuck */

  if (ret) {
    DMB;
    return ret;
  }

  if ((i & (CMD_DONE_MASK|ERR_MASK)) != CMD_DONE_MASK) {
    if (arasan_sd_readreg(io, EMMC_STATUS_REG_OFFSET) & CMD_INHIBIT_MASK) {
      if (0 != (ret = arasan_sd_reset_circuit(io, SRST_CMD_MASK))) {
        DMB;
        return ret;
      }
    }
    DMB;
    return (i & CTO_ERR_MASK) ? -EBUSY : -EIO;
  }

  switch (c & CMD_RSPNS_TYPE_MASK) {
    case CMD_NO_RESP:
      resp[0] = 0;
      break;
    case CMD_136BIT_RESP:
      /* XXX: this feels wrong */
      i = arasan_sd_readreg(io, EMMC_RESP0_REG_OFFSET);
      v = arasan_sd_readreg(io, EMMC_RESP1_REG_OFFSET);
      resp[0] = (i << 8);
      resp[1] = (i >> 24) | (v << 8);
      i = arasan_sd_readreg(io, EMMC_RESP2_REG_OFFSET);
      resp[2] = (v >> 24) | (i << 8);
      v = arasan_sd_readreg(io, EMMC_RESP3_REG_OFFSET);
      resp[3] = (i >> 24) | (v << 8);
      break;
    case CMD_BUSY48BIT_RESP:  /* intentional fallthrough */
    case CMD_48BIT_RESP:
      resp[0] = arasan_sd_readreg(io, EMMC_RESP0_REG_OFFSET);
      break;
  }

  if ((c & CMD_RSPNS_TYPE_MASK) == CMD_BUSY48BIT_RESP) {
    v = arasan_sd_readreg(io, EMMC_IRPT_EN_REG_OFFSET);
    arasan_sd_writereg(io,
      EMMC_IRPT_EN_REG_OFFSET, v & ~(DATA_DONE_MASK|ERR_MASK));
    c = arasan_sd_readreg(io, EMMC_IRPT_MASK_REG_OFFSET);
    arasan_sd_writereg(io,
      EMMC_IRPT_MASK_REG_OFFSET, c | (DATA_DONE_MASK|ERR_MASK));
    ret = arasan_sd_await_mask_in_reg(io,
      EMMC_INTERRUPT_REG_OFFSET, DATA_DONE_MASK|ERR_MASK, &i, 1000000);
    arasan_sd_writereg(io,
      EMMC_INTERRUPT_REG_OFFSET, i & (DATA_DONE_MASK|ERR_MASK));
    arasan_sd_writereg(io, EMMC_IRPT_MASK_REG_OFFSET, c);
    arasan_sd_writereg(io, EMMC_IRPT_EN_REG_OFFSET, v);
    if (ret) {
      DMB;
      return ret;
    }
    if ((i & (DATA_DONE_MASK|ERR_MASK)) != DATA_DONE_MASK) {
      DMB;
      return (i & DTO_ERR_MASK) ? -EBUSY : -EIO;
    }
  }

  if (cmd == SD_CMD_SELECT_CARD ||
      cmd == SD_CMD_VOLTAGE_SWITCH ||
      cmd == SD_CMD_SWITCH_FUNC ||
      cmd == SD_ACMD_SET_BUS_WIDTH) {
    if (!SD_CARD_STATUS_IS_ERROR(resp[0])) {
      if (cmd == SD_CMD_SELECT_CARD) {
        if (0 != (ret = arasan_sd_setclk(io, FREQ_NORMAL))) {
          DMB;
          return ret;
        }
        io->fast = 1;
      } else if (cmd == SD_CMD_VOLTAGE_SWITCH) {
        /* disable the clock */
        arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET,
          arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET) & ~(CLK_EN_MASK));

        /* if any DAT line is set we have a problem */
        if (arasan_sd_readreg(io, EMMC_STATUS_REG_OFFSET) & 0x00f00000) {
          DMB;
          return -ENXIO;
        }

        /* set the 1v8 flag and ensure that it sticks */
        arasan_sd_writereg(io, EMMC_CONTROL0_REG_OFFSET,
          arasan_sd_readreg(io, EMMC_CONTROL0_REG_OFFSET) | (ENABLE_1_8V_MASK));
        udelay(5);
        if (!(arasan_sd_readreg(io,
            EMMC_CONTROL0_REG_OFFSET) & (ENABLE_1_8V_MASK))) {
          DMB;
          return -ENXIO;
        }

        /* enable the clock */
        arasan_sd_writereg(io, EMMC_CONTROL1_REG_OFFSET,
          arasan_sd_readreg(io, EMMC_CONTROL1_REG_OFFSET) | (CLK_EN_MASK));
        udelay(1);

        if (0 != (ret = arasan_sd_await_mask_in_reg(io,
            EMMC_CONTROL1_REG_OFFSET, CLK_STABLE_MASK, NULL, 1000))) {
          DMB;
          return ret;
        }

        /* if any DAT line is set we're good */
        if (!(arasan_sd_readreg(io, EMMC_STATUS_REG_OFFSET) & 0x00f00000)) {
          DMB;
          return -ENXIO;
        }
      } else if (cmd == SD_CMD_SWITCH_FUNC) {
        /* TODO: this is rubbish - we can do better */
        if ((arg & 0x8000000f) == 0x80000001) {
          if (0 != (ret = arasan_sd_setclk(io, FREQ_HS))) {
            DMB;
            return ret;
          }
        }
      } else if (cmd == SD_ACMD_SET_BUS_WIDTH) {
        v = arasan_sd_readreg(io, EMMC_CONTROL0_REG_OFFSET);
        switch (arg) {
          case 0:
            arasan_sd_writereg(io,
              EMMC_CONTROL0_REG_OFFSET, v & ~HCTL_DWIDTH_MASK);
            break;
          case 2:
            arasan_sd_writereg(io,
              EMMC_CONTROL0_REG_OFFSET, v | HCTL_DWIDTH_MASK);
            break;
        }
      }
    }
  }

  DMB;
  return 0;
}

static int arasan_sd_io_iosetup(uptr_t handle, u16 s, u16 c)
{
  struct arasan_sd_io_t *io;
  u32 v;
  int ret;

  if (!(io = from_uptr(handle)))
    return -ENODEV;

  v = (c << BLKCNT_SHIFT) | (s << BLKSIZE_SHIFT);
  if (v & BLKSIZECNT_RESERVED_MASK)
    return -EINVAL;

  DMB;
  if (0 != (ret = arasan_sd_await_mask_not_in_reg(io,
      EMMC_STATUS_REG_OFFSET, DAT_INHIBIT_MASK, NULL, 500000))) {
    DMB;
    return ret;
  }

  arasan_sd_writereg(io, EMMC_BLKSIZECNT_REG_OFFSET, v);
  DMB;
  return 0;
}

static int arasan_sd_io_iostart(uptr_t handle, int w, int a, u32 ba, u8 *buf, u32 len)
{
  struct arasan_sd_io_t *io;
  int ret;
  u32 resp[4];
  u32 cmd;
  u32 v;
  u32 i;

  if (!(io = from_uptr(handle)))
    return -ENODEV;

  DMB;

  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET,
    arasan_sd_readreg(io, EMMC_INTERRUPT_REG_OFFSET));

  /* TODO: maybe reset data circuitry? */
  if (0 != (ret = arasan_sd_await_mask_not_in_reg(io,
      EMMC_STATUS_REG_OFFSET, DAT_INHIBIT_MASK, NULL, 500000))) {
    DMB;
    return ret;
  }

  cmd = w ? SD_CMD_WRITE_BLOCK : SD_CMD_READ_SINGLE_BLOCK;
  if (0 != (ret = arasan_sd_io_cmd(handle, cmd, ba, resp))) {
    DMB;
    return ret;
  }

  v = DATA_DONE_MASK|ERR_MASK;
  if (w) {
    for (i = 0; i < len; i += 4) {
      if (0 != (ret = arasan_sd_await_mask_in_reg(io,
          EMMC_STATUS_REG_OFFSET, WRITE_TRANSFER_MASK, NULL, 1000))) {
        DMB;
        return ret;
      }
      arasan_sd_writereg(io, EMMC_DATA_REG_OFFSET,
        ((buf[i + 0] <<  0) & 0x000000ff) |
        ((buf[i + 1] <<  8) & 0x0000ff00) |
        ((buf[i + 2] << 16) & 0x00ff0000) |
        ((buf[i + 3] << 24) & 0xff000000));
    }
  } else
    v |= READ_RDY_MASK;

  arasan_sd_writereg(io,
    a ? EMMC_IRPT_EN_REG_OFFSET : EMMC_IRPT_MASK_REG_OFFSET, v);

  DMB;
  return 0;
}

static int arasan_sd_io_iowait(uptr_t handle, int w, u32 max_us)
{
  struct arasan_sd_io_t *io;
  u32 v;
  int ret;

  if (!(io = from_uptr(handle)))
    return -ENODEV;

  v = DATA_DONE_MASK|ERR_MASK;
  v |= w ? WRITE_RDY_MASK : READ_RDY_MASK;

  DMB;
  ret = arasan_sd_await_mask_in_reg(io,
    EMMC_INTERRUPT_REG_OFFSET, v, NULL, max_us);
  DMB;
  return ret;
}

static int arasan_sd_io_iofinish(uptr_t handle, int w, u8 *buf, u32 len)
{
  struct arasan_sd_io_t *io;
  u32 i;
  u32 j;
  int ret;
  u32 *data;

  if (!(io = from_uptr(handle)))
    return -ENODEV;

  DMB;

  i = arasan_sd_readreg(io, EMMC_INTERRUPT_REG_OFFSET);

  if (i & ERR_MASK) {
    arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, i);
    DMB;
    return -EIO;
  }

  if (!w) {
    data = (u32 *)buf;
    for (j = 0; j < len/4; ++j) {
      if (0 != (ret = arasan_sd_await_mask_in_reg(io,
          EMMC_STATUS_REG_OFFSET, READ_TRANSFER_MASK, NULL, 1000))) {
        DMB;
        return ret;
      }
      data[j] = arasan_sd_readreg(io, EMMC_DATA_REG_OFFSET);
    }
  }

  arasan_sd_writereg(io, EMMC_IRPT_EN_REG_OFFSET, 0x0U);
  arasan_sd_writereg(io, EMMC_INTERRUPT_REG_OFFSET, i);
  DMB;
  return 0;
}

const struct sd_io_interface_t sd_io_interface = {
  arasan_sd_io_init,
  arasan_sd_io_fini,
  arasan_sd_io_cmd,
  arasan_sd_io_iosetup,
  arasan_sd_io_iostart,
  arasan_sd_io_iowait,
  arasan_sd_io_iofinish,
  (OCR_3V2TO3V3|OCR_3V3TO3V4),  /* ocr_supported */
  1,                            /* hs_capable */
  1,                            /* hc_capable */
  0,                            /* switch_1v8_capable */
  1,                            /* four_bit_bus_capable */
};
