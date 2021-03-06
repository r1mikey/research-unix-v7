#include "bcm283x_mbox.h"

#include "bcm283x_io.h"
#include "../arm1176jzfs.h"
#include "../bcm283x_machdep.h"

#include "../kstddef.h"
#include "../../h/types.h"

#define MBOX_OFFSET                   0x0000b880
#define MBOX_BASE                     ((_bcm283x_iobase) + (MBOX_OFFSET))

#define MBOX0_RW_REG_OFFSET           0x00000000
#define MBOX0_PEEK_REG_OFFSET         0x00000010
#define MBOX0_SENDER_REG_OFFSET       0x00000014
#define MBOX0_STATUS_REG_OFFSET       0x00000018
#define MBOX0_CONFIG_REG_OFFSET       0x0000001c
#define MBOX1_RW_REG_OFFSET           0x00000020
#define MBOX1_PEEK_REG_OFFSET         0x00000030
#define MBOX1_SENDER_REG_OFFSET       0x00000034
#define MBOX1_STATUS_REG_OFFSET       0x00000038
#define MBOX1_CONFIG_REG_OFFSET       0x0000003c

#define MBOX0_RW_REG                  ((MBOX_BASE) + (MBOX0_RW_REG_OFFSET))
#define MBOX0_PEEK_REG                ((MBOX_BASE) + (MBOX0_PEEK_REG_OFFSET))
#define MBOX0_SENDER_REG              ((MBOX_BASE) + (MBOX0_SENDER_REG_OFFSET))
#define MBOX0_STATUS_REG              ((MBOX_BASE) + (MBOX0_STATUS_REG_OFFSET))
#define MBOX0_CONFIG_REG              ((MBOX_BASE) + (MBOX0_CONFIG_REG_OFFSET))
#define MBOX1_RW_REG                  ((MBOX_BASE) + (MBOX1_RW_REG_OFFSET))
#define MBOX1_PEEK_REG                ((MBOX_BASE) + (MBOX1_PEEK_REG_OFFSET))
#define MBOX1_SENDER_REG              ((MBOX_BASE) + (MBOX1_SENDER_REG_OFFSET))
#define MBOX1_STATUS_REG              ((MBOX_BASE) + (MBOX1_STATUS_REG_OFFSET))
#define MBOX1_CONFIG_REG              ((MBOX_BASE) + (MBOX1_CONFIG_REG_OFFSET))

#define MBOX_STATUS_FULL_BIT          31
#define MBOX_STATUS_EMPTY_BIT         30

#define MBOX_STATUS_FULL_MASK         BIT(MBOX_STATUS_FULL_BIT)
#define MBOX_STATUS_EMPTY_MASK        BIT(MBOX_STATUS_EMPTY_BIT)

#define MBOX_PROP_CHAN_ARM_TO_VC      8
#define MBOX_PROP_CHAN_VC_TO_ARM      9

/*
 * mbox_buffer[0]: buffer size in bytes
 * mbox_buffer[1]: request/response code
 *                  REQ:
 *                   Process Request: 0x00000000
 *                  RSP:
 *                   Request Success: 0x80000000
 *                   Parse Error    : 0x80000001
 * mbox_buffer[2..]: concatenated tags
 * mbox_buffer[-1]: End tag: 0x0
 * mbox_buffer[..]: Padding (0x0)
 *
 * Tag Format
 * ==========
 *
 * u32: tag identifier
 * u32: value buffer size in bytes
 * u32:
 *   Request codes:
 *     b31 clear: request
 *     b30-b0: reserved
 *   Response codes:
 *     b31 set: response
 *     b30-b0: value length in bytes
 * u8...: value buffer
 * u8...: padding to align the tag to 32 bits.
 */

#define MBOX_REQUEST_PROCESS          0x00000000
#define MBOX_RESPONSE_SUCCESS         0x80000000
#define MBOX_RESONSE_PARSE_ERROR      0x80000001

#define MBOX_TAG_REQUEST_CODE         0x00000000
#define MBOX_TAG_RESPONSE_CODE        0x80000000
#define MBOX_TAG_GETSERIAL            0x00010004
#define MBOX_TAG_ARM_MEMORY           0x00010005
#define MBOX_TAG_GETCLKRATE           0x00030002
#define MBOX_TAG_SETCLKRATE           0x00038002
#define MBOX_TAG_SETPOWERSTATE        0x00028001
#define MBOX_TAG_END                  0x00000000

#define MBOX_PERIPHERAL_SDCARD        0x00000000
#define MBOX_CLOCK_EMMC               0x00000001
#define MBOX_CLOCK_UART               0x00000002

#define MBOX_BUFFER_SIZE              36
#define MBOX_BUFFER_BYTES             (MBOX_BUFFER_SIZE * 4)
static volatile u32  __attribute__((aligned(16))) mbox_buffer[MBOX_BUFFER_SIZE];

static void bcm283x_reset_mbox_buffer(void)
{
  int i;

  mbox_buffer[0] = MBOX_BUFFER_BYTES;
  mbox_buffer[1] = MBOX_REQUEST_PROCESS;

  for (i = 2; i < MBOX_BUFFER_SIZE; ++i)
    mbox_buffer[i] = MBOX_TAG_END;
}


/*
 * ```The ARM should never write MB 0 or read MB 1.```
 * ergo, read from MB0, write to MB1
 * See https://github.com/raspberrypi/firmware/wiki/Mailboxes
 */
static u32 bcm283x_mbox_read(u8 chan)
{
  u32 v;

  while (1) {
    while (ioread32(MBOX0_STATUS_REG) & MBOX_STATUS_EMPTY_MASK);
    v = ioread32(MBOX0_RW_REG);
    if ((v & 0xf) == chan)
      break;
  }

  return v & 0xfffffff0;
}


static void bcm283x_mbox_write(u8 chan, u32 v)
{
  while (ioread32(MBOX1_STATUS_REG) & MBOX_STATUS_FULL_MASK);
  iowrite32(MBOX1_RW_REG, (v & 0xfffffff0) | (chan & 0xf));
}


static int bcm283x_mbox_write_then_read(u8 chan)
{
  u32 v;
  u32 s;
  u32 e;

  s = (u32)&mbox_buffer[0];
  e = s + (sizeof(mbox_buffer) - 1);
  dcachecva(s, e);
  DSB;
  bcm283x_mbox_write(chan, MEM_VIRT_TO_PHYS(&mbox_buffer[0]));
  DSB;
  v = bcm283x_mbox_read(chan);
  dcacheiva(s, e);
  DSB;

  if (v != MEM_VIRT_TO_PHYS(&mbox_buffer[0]))
    return -1;  /* -EBADMSG */

  if (mbox_buffer[1] != MBOX_RESPONSE_SUCCESS)
    return -1;  /* -EBADMSG */

  return 0;
}


/*
 * This is technically wrong, but c'est la vie.
 */
int bcm283x_mbox_get_arm_memory(u32 *v)
{
  bcm283x_reset_mbox_buffer();

  mbox_buffer[2] = MBOX_TAG_ARM_MEMORY;      /* tag identifier */
  mbox_buffer[3] = 32;                       /* value buffer size in bytes */
  mbox_buffer[4] = MBOX_TAG_REQUEST_CODE;    /* request code: b31 clear, request */
  mbox_buffer[5] = 0x0;                      /* base 0 */
  mbox_buffer[6] = 0x0;                      /* size 0 */
  mbox_buffer[7] = 0x0;                      /* base 1 */
  mbox_buffer[8] = 0x0;                      /* size 1 */
  mbox_buffer[9] = 0x0;                      /* base 2 */
  mbox_buffer[10] = 0x0;                     /* size 2 */
  mbox_buffer[11] = 0x0;                     /* base 3 */
  mbox_buffer[12] = 0x0;                     /* size 3 */

  if (0 != bcm283x_mbox_write_then_read(MBOX_PROP_CHAN_ARM_TO_VC))
    return -1;

  *v = mbox_buffer[6];
  return 0;
}


int bcm283x_mbox_set_uart_clock(u32 hz, u32 *new_hz)
{
  bcm283x_reset_mbox_buffer();

  mbox_buffer[2] = MBOX_TAG_SETCLKRATE;     /* tag identifier */
  mbox_buffer[3] = 12;                      /* value buffer size in bytes */
  mbox_buffer[4] = MBOX_TAG_REQUEST_CODE;   /* request code: b31 clear, request */
  mbox_buffer[5] = MBOX_CLOCK_UART;         /* clock id: UART clock */
  mbox_buffer[6] = hz;                      /* value buffer */
  mbox_buffer[7] = 0;                       /* clear turbo */

  if (0 != bcm283x_mbox_write_then_read(MBOX_PROP_CHAN_ARM_TO_VC))
    return -1;

  if (!mbox_buffer[6])
    return -1;

  if (new_hz)
    *new_hz = mbox_buffer[6];

  return 0;
}


int bcm283x_mbox_get_sdcard_clock(u32 *hz)
{
  bcm283x_reset_mbox_buffer();

  mbox_buffer[2] = MBOX_TAG_GETCLKRATE;     /* tag identifier */
  mbox_buffer[3] = 12;                      /* value buffer size in bytes */
  mbox_buffer[4] = MBOX_TAG_REQUEST_CODE;   /* request code: b31 clear, request */
  mbox_buffer[5] = MBOX_CLOCK_EMMC;         /* clock id: EMMC clock */
  mbox_buffer[6] = 0x0;                     /* rate */
  mbox_buffer[7] = 0x0;                     /* turbo */

  if (0 != bcm283x_mbox_write_then_read(MBOX_PROP_CHAN_ARM_TO_VC))
    return -1;

  if (!mbox_buffer[6])
    return -1;

  if (hz)
    *hz = mbox_buffer[6];

  return 0;
}


int bcm283x_mbox_sdcard_power(u32 onoff)
{
  bcm283x_reset_mbox_buffer();

  mbox_buffer[2] = MBOX_TAG_SETPOWERSTATE;     /* tag identifier */
  mbox_buffer[3] = 8;                          /* value buffer size in bytes */
  mbox_buffer[4] = MBOX_TAG_REQUEST_CODE;      /* request code: b31 clear, request */
  mbox_buffer[5] = MBOX_PERIPHERAL_SDCARD;     /* peripheral id: SD Card */
  mbox_buffer[6] = 0x02|(onoff ? 1 : 0);       /* value buffer */

  if (0 != bcm283x_mbox_write_then_read(MBOX_PROP_CHAN_ARM_TO_VC))
    return -1;

  if (mbox_buffer[5] != MBOX_PERIPHERAL_SDCARD)
    return -1;

  if (onoff) {
    if (!(mbox_buffer[6] & 0x01))
      return -1;
  } else {
    if (mbox_buffer[6] & 0x01)
      return -1;
  }

  return 0;
}
