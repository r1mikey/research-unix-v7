#include "sd_io.h"

#include "../../h/param.h"
#include "../../h/types.h"
#include "../../h/user.h"
#include "../../h/prf.h"

#include "bcm283x_systimer.h"

#define CARD_SD_TYPE_UNKNOWN    0
#define CARD_SD_TYPE_1          1
#define CARD_SD_TYPE_2_SC       2
#define CARD_SD_TYPE_2_HC       3

#define SCR_LO_SD_SPEC_MASK             0x0000000f
#define SCR_LO_SD_SPEC_SHIFT            0
#define SCR_LO_SD_SPEC_1_101            (0x0U << SCR_LO_SD_SPEC_SHIFT)
#define SCR_LO_SD_SPEC_11               (0x1U << SCR_LO_SD_SPEC_SHIFT)
#define SCR_LO_SD_SPEC_2_3              (0x2U << SCR_LO_SD_SPEC_SHIFT)
#define SCR_LO_STRUCT_VER_MASK          0x000000f0
#define SCR_LO_STRUCT_VER_SHIFT         4
#define SCR_LO_VER_1                    (0x0U << SCR_LO_STRUCT_VER_SHIFT)
#define SCR_LO_BUS_WIDTH_MASK           0x00000f00
#define SCR_LO_BUS_WIDTH_SHIFT          8
#define SCR_LO_BUS_WIDTH_1              (0x1U << SCR_LO_BUS_WIDTH_SHIFT)
#define SCR_LO_BUS_WIDTH_4              (0x4U << SCR_LO_BUS_WIDTH_SHIFT)
#define SCR_LO_SD_SECURITY_MASK         0x00007000
#define SCR_LO_SD_SECURITY_SHIFT        12
#define SCR_LO_SD_SEC_NONE              (0x0U << SCR_LO_SD_SECURITY_SHIFT)
#define SCR_LO_SD_SEC_NOT_USED          (0x1U << SCR_LO_SD_SECURITY_SHIFT)
#define SCR_LO_SD_SEC_101               (0x2U << SCR_LO_SD_SECURITY_SHIFT)
#define SCR_LO_SD_SEC_2                 (0x3U << SCR_LO_SD_SECURITY_SHIFT)
#define SCR_LO_SD_SEC_3                 (0x4U << SCR_LO_SD_SECURITY_SHIFT)
#define SCR_LO_DATA_AFTER_ERASE_SHIFT   15
#define SCR_LO_DATA_AFTER_ERASE_MASK    (0x1U << SCR_LO_DATA_AFTER_ERASE_SHIFT)
#define SCR_LO_DATA_AFTER_ERASE         (SCR_LO_DATA_AFTER_ERASE_MASK)
#define SCR_LO_EX_SECURITY_MASK         0x00780000
#define SCR_LO_EX_SECURITY_SHIFT        19
#define SCR_LO_EX_SEC_NONE              (0x0U << SCR_LO_EX_SECURITY_SHIFT)
#define SCR_LO_SD_SPEC3_SHIFT           23
#define SCR_LO_SD_SPEC3_MASK            (0x1U << SCR_LO_SD_SPEC3_SHIFT)
#define SCR_LO_SD_SPEC3                 (SCR_LO_SD_SPEC3_MASK)
#define SCR_LO_CMD_SUPPORT_MASK         0x03000000
#define SCR_LO_CMD_SUPPORT_SHIFT        24
#define SCR_LO_CMD_SUPP_SPEED_CLASS     (0x1U << SCR_LO_CMD_SUPPORT_SHIFT)
#define SCR_LO_CMD_SUPP_SET_BLKCNT      (0x2U << SCR_LO_CMD_SUPPORT_SHIFT)

struct sd_io_instance_t {
  uptr_t base;
  uptr_t io_handle;
  u32 card_type;
  u32 ocr;
  u32 cid[4];
  u32 csd[4];
  u32 scr[2];  /* low, high */
  u32 sector_size;
  u32 num_sectors;
  u32 rca;
};

#define NUM_SD_IO 1
struct sd_io_instance_t sd_io_instance[NUM_SD_IO];

static struct sd_io_instance_t * sd_io_instance_get(uptr_t iobase)
{
  struct sd_io_instance_t *sd;
  int i;
 
  for (i = 0; i < NUM_SD_IO; ++i) {
    if (sd_io_instance[i].base == iobase) {
      return &sd_io_instance[i];
    }
  }
 
  for (i = 0; i < NUM_SD_IO; ++i) {
    if (sd_io_instance[i].base == 0) {
      sd = &sd_io_instance[i];
      sd->base = iobase;
      break;
    }
  }
 
  if (i >= NUM_SD_IO)
    return NULL;
 
  return sd;
}

static void sd_io_instance_put(struct sd_io_instance_t *sd)
{
  int i;

  for (i = 0; i < NUM_SD_IO; ++i) {
    if (sd == &sd_io_instance[i]) {
      sd->base = 0;
      sd->io_handle = 0;
      sd->card_type = CARD_SD_TYPE_UNKNOWN;
      sd->ocr = 0;
      sd->cid[0] = sd->cid[1] = sd->cid[2] = sd->cid[3] = 0;
      sd->csd[0] = sd->csd[1] = sd->csd[2] = sd->csd[3] = 0;
      sd->rca = 0;
      break;
    }
  }
}

static struct sd_io_instance_t * from_uptr(uptr_t handle)
{
  int i;

  for (i = 0; i < NUM_SD_IO; ++i) {
    if (handle == (uptr_t)(&sd_io_instance[i])) {
      return &sd_io_instance[i];
    }
  }

  return NULL;
}

static uptr_t to_uptr(struct sd_io_instance_t *sd)
{
  return (uptr_t)sd;
}

static void sd_io_instance_fini(uptr_t handle)
{
  struct sd_io_instance_t *sd;
 
  if (!handle || !(sd = from_uptr(handle)))
    return;

  if (sd->io_handle)
    sd_io_interface.fini(sd->io_handle);
  sd_io_instance_put(sd);
}

/*
 * XXX: this is very, very likely wrong
 */
static int parse_csd(struct sd_io_instance_t *sd)
{
  u32 csize;
  u32 mult;

  /*
   * [0] -- 0  to 31
   * [1] -- 32 to 63
   * [2] -- 64 to 95
   * [3] -- 96 to 127
   */
  sd->sector_size = 1U << ((sd->csd[2] >> 16) & 0xf);

  switch ((sd->csd[3] >> 30) & 0x3) {
    case 0:  /* CSD version 1 */
      /* XXX: CSDv1 is untested */
      csize = ((sd->csd[1] >> 30) & 0x3);
      csize |= ((sd->csd[2] & 0x3ff) << 2);
      mult = (sd->csd[1] >> 15) & 0x7;
      sd->num_sectors = (csize + 1) * (1U << (mult + 2));
      break;
    case 1:  /* CSD version 2 */
      csize = (sd->csd[1] >> 16) & 0xffff;
      csize |= ((sd->csd[2] & 0xff) << 16);
      sd->num_sectors = (csize + 1) * 0x80000LLU / sd->sector_size;
      break;
  }

  if (sd->sector_size == 1024) {
    sd->num_sectors <<= 1;
    sd->sector_size = 512;
  }

  return 0;
}

static int sd_io_instance_cmd(struct sd_io_instance_t *sd, u32 cmd, u32 arg, u32 *resp)
{
  int ret;
  u32 card_status;

  if (!(SD_CMD_IS_VALID(cmd)))
    return -EINVAL;

  if (IS_SD_ACMD(cmd)) {
    if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_APP_CMD, sd->rca, resp)))
      return ret;
    if (!(resp[0] & (SD_CARD_STATUS_APP_CMD)))
      return -ENXIO;
  }

  if (0 != (ret = sd_io_interface.cmd(sd->io_handle, cmd, arg, resp)))
    return ret;

  switch (SD_CMD_GET_RSP_TYPE(cmd)) {
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R1):  /* intentional fallthrough */
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R1B):
      card_status = resp[0] & (SD_CARD_STATUS_READ_MASK);
      if (card_status & (SD_CARD_STATUS_ERROR_MASK))
        return -EIO;
      break;
    case SD_CMD_GET_RSP_TYPE(SD_RESP_TYPE_R6):
      card_status = (SD_CARD_STATUS_R6_TO_CARD_STATUS(resp[0])) & (SD_CARD_STATUS_READ_MASK);
      if (card_status & (SD_CARD_STATUS_ERROR_MASK))
        return -EIO;
      break;
    default:
      break;
  }

  return 0;
}

static uptr_t sd_io_instance_init(uptr_t iobase, u32 extclk)
{
  struct sd_io_instance_t *sd;
  u32 resp[4];
  int ret;
  u32 i;
  u32 v2_or_later;
  u32 sfbuf[16];
  u32 acmd41_req;

  if (!(sd = sd_io_instance_get(iobase)))
    goto out;

  if (!(sd->io_handle = sd_io_interface.init(iobase, extclk)))
    goto out;

  /*
   * See Physical Layer Simplified Specification Version 7.10 page 58 (38),
   * Figure 4.2: Card Initialization and Identification Flow (SD mode).
   *
   * Need to pick up on 'SDIO Card Specification' at some stage.
   */

  /* CMD0 */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_GO_IDLE_STATE, 0, resp)))
    goto out;

  /* CMD8 2.7-3.6V, 0xaa as a check pattern, no PCIe stuff */
  ret = sd_io_instance_cmd(sd, SD_CMD_SEND_IF_COND, 0x000001aa, resp);

  switch (ret) {
    case 0:
      v2_or_later = 1;
      if (resp[0] != 0x000001aa)
        goto out;
      acmd41_req = (SD_SEND_OP_COND_XPC);
      if (sd_io_interface.switch_1v8_capable)
        acmd41_req |= (SD_SEND_OP_COND_S18R);
      if (sd_io_interface.hc_capable)
        acmd41_req |= (SD_SEND_OP_COND_HCS);
      break;
    case -EBUSY:
      acmd41_req = 0;
      v2_or_later = 0;
      break;
    default:
      goto out;
  }

  /* ACMD41 */
  /* start with an inquiry */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_ACMD_SD_SEND_OP_COND, 0x0, resp)))
    goto out;
  acmd41_req |= (resp[0] & sd_io_interface.ocr_supported);

  if (!v2_or_later) {
    for (i = 0; i < 120; ++i) {
      if (0 != (ret = sd_io_instance_cmd(sd, SD_ACMD_SD_SEND_OP_COND, acmd41_req, resp)))
        goto out;
      /* TODO: check to see of the card has gone inactive */
      if (resp[0] & OCR_CARD_POWER_UP_BUSY)
        break;
      udelay(10000);
    }
    if (i >= 120) {
      ret = -EIO;
      goto out;
    }
    sd->ocr = resp[0];
    sd->card_type = CARD_SD_TYPE_1;
  } else {
    for (i = 0; i < 120; ++i) {
      if (0 != (ret = sd_io_instance_cmd(sd, SD_ACMD_SD_SEND_OP_COND, acmd41_req, resp)))
        goto out;
      if (resp[0] & OCR_CARD_POWER_UP_BUSY)
        break;
      udelay(10000);
    }
    if (i >= 120) {
      ret = -EIO;
      goto out;
    }
    sd->ocr = resp[0];

    sd->card_type = (resp[0] & (SD_SEND_OP_COND_CCS)) ? CARD_SD_TYPE_2_HC : CARD_SD_TYPE_2_SC;
  }

  if (sd->card_type == CARD_SD_TYPE_2_HC && sd->ocr & SD_SEND_OP_COND_S18A &&
      acmd41_req & SD_SEND_OP_COND_S18R) {
    if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_VOLTAGE_SWITCH, 0, resp)))
      goto out;  /* TODO: we could initiate a retry withoout 1v8 here */
  }

  /* CMD2 */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_ALL_SEND_CID, 0, sd->cid)))
    goto out;
 
  /* CMD3 */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_SEND_RELATIVE_ADDR, 0, resp)))
    goto out;
  sd->rca = resp[0] & 0xffff0000;

  /*
   * This CMD9 SEND_CSD at this point is non-standard, but the CSD must be
   * grabbed in STBY state, and the CMD7(SELECT_CARD) that follows takes the card
   * from STBY to TRAN.
   */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_SEND_CSD, sd->rca, sd->csd)))
    goto out;
  if (0 != (ret = parse_csd(sd)))
    goto out;

  /* CMD7 */
  if (0 != (ret = sd_io_instance_cmd(sd, SD_CMD_SELECT_CARD, sd->rca, resp)))
    goto out;

  /* CMD42: SD_CMD_LOCK_UNLOCK - based on status bits from the preceding CMD7 */

  if (sd->card_type != CARD_SD_TYPE_2_HC)
    if (0 != (ret = sd_io_instance_cmd(sd,
        SD_CMD_SET_BLOCKLEN, sd->sector_size, resp)))
      goto out;

  /* read the SCR - moves from TRAN to DATA */
  if (0 != (ret = sd_io_interface.iosetup(sd->io_handle, 8, 1)))
    goto out;
  if (0 != (ret = sd_io_instance_cmd(sd, SD_ACMD_SEND_SCR, 0, resp)))
    goto out;
  if (0 != (ret = sd_io_interface.iowait(sd->io_handle, 0, 500000)))
    goto out;
  if (0 != (ret = sd_io_interface.iofinish(sd->io_handle, 0, (u8 *)sd->scr, 8)))
    goto out;

  /* ACMD6: SD_ACMD_SET_BUS_WIDTH (we needed the SCR for this) */
  if (sd_io_interface.four_bit_bus_capable && sd->scr[0] & SCR_LO_BUS_WIDTH_4)
    if (0 != (ret = sd_io_instance_cmd(sd, SD_ACMD_SET_BUS_WIDTH, 0x2, resp)))
      goto out;

  /*
   * TODO: handle UHS-I init sequence properly:
   *  CMD6, FG3    -- Driver Strength
   *  CMD6, FG1,4  -- UHS-I Mode, Power Limit
   *  CMD19 (tuning loop)
   */

out:
  if (!sd)
    return 0;

  if (!sd->io_handle) {
    sd_io_instance_fini(to_uptr(sd));
    return 0;
  }

  if (ret) {
    sd_io_instance_fini(to_uptr(sd));
    return 0;
  }

  return to_uptr(sd);
}

static int sd_io_iostart(uptr_t handle, int w, int a, u32 ba, u8 *buf, u32 len)
{
  struct sd_io_instance_t *sd;
  int ret;
 
  if (!(sd = from_uptr(handle)))
    return -ENODEV;

  if (0 != (ret = sd_io_interface.iosetup(sd->io_handle, sd->sector_size, 1)))
    return ret;

  if (sd->card_type == CARD_SD_TYPE_2_SC)
    ba <<= 9;

  if (0 != (ret = sd_io_interface.iostart(sd->io_handle, w, a, ba, buf, len)))
    return ret;

  return 0;
}

static int sd_io_iowait(uptr_t handle, int w, u32 max_us)
{
  struct sd_io_instance_t *sd;
 
  if (!(sd = from_uptr(handle)))
    return -ENODEV;

  return sd_io_interface.iowait(sd->io_handle, w, max_us);
}

static int sd_io_iofinish(uptr_t handle, int w, u8 *buf, u32 len)
{
  struct sd_io_instance_t *sd;
 
  if (!(sd = from_uptr(handle)))
    return -ENODEV;

  return sd_io_interface.iofinish(sd->io_handle, w, buf, len);
}

const struct sd_io_t sd_io = {
  sd_io_instance_init,
  sd_io_instance_fini,
  sd_io_iostart,
  sd_io_iowait,
  sd_io_iofinish,
};
