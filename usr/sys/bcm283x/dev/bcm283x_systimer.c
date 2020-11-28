#include "bcm283x_systimer.h"

#include "bcm283x_io.h"
#include "bcm283x_irq.h"
#include "../arm1176jzfs.h"
#include "../trap.h"
/* #include "irq.h" */
/* #include "util.h" */
#include "../../h/types.h"
#include "../../h/param.h"


#define SYSTMR_OFFSET                 0x00003000
#define SYSTMR_BASE                   ((_bcm283x_iobase) + (SYSTMR_OFFSET))
#define ARMCTRL_OFFSET                0x01000000
#define ARMCTRL_BASE                  ((_bcm283x_iobase) + (ARMCTRL_OFFSET))

#define SYSTMR_CS_REG_OFFSET          0x00000000  /* control and status      */
#define SYSTMR_CLO_REG_OFFSET         0x00000004  /* free-running low bits   */
#define SYSTMR_CHI_REG_OFFSET         0x00000008  /* free-running high bits  */
#define SYSTMR_C0_REG_OFFSET          0x0000000c  /* used by GPU             */
#define SYSTMR_C1_REG_OFFSET          0x00000010  /* sched timer             */
#define SYSTMR_C2_REG_OFFSET          0x00000014  /* used by GPU             */
#define SYSTMR_C3_REG_OFFSET          0x00000018  /* profiling timer?        */

#define SYSTMR_CS_REG                 ((SYSTMR_BASE) + (SYSTMR_CS_REG_OFFSET))
#define SYSTMR_CLO_REG                ((SYSTMR_BASE) + (SYSTMR_CLO_REG_OFFSET))
#define SYSTMR_CHI_REG                ((SYSTMR_BASE) + (SYSTMR_CHI_REG_OFFSET))
#define SYSTMR_C0_REG                 ((SYSTMR_BASE) + (SYSTMR_C0_REG_OFFSET))
#define SYSTMR_C1_REG                 ((SYSTMR_BASE) + (SYSTMR_C1_REG_OFFSET))
#define SYSTMR_C2_REG                 ((SYSTMR_BASE) + (SYSTMR_C2_REG_OFFSET))
#define SYSTMR_C3_REG                 ((SYSTMR_BASE) + (SYSTMR_C3_REG_OFFSET))

#define SYSTMR_CS_M0                  0x00000001  /* system timer match 0    */
#define SYSTMR_CS_M1                  0x00000002  /* system timer match 1    */
#define SYSTMR_CS_M2                  0x00000004  /* system timer match 2    */
#define SYSTMR_CS_M3                  0x00000008  /* system timer match 3    */

#define LOCAL_TMR_CTRL_REG_OFFSET     0x00000034
#define LOCAL_TMR_STAT_REG_OFFSET     0x00000034
#define LOCAL_TMR_CLR_REG_OFFSET      0x00000038
#define LOCAL_TMR_IRQ_RT_OFFSET       0x00000024

#define LOCAL_TMR_CTRL_REG            ((ARMCTRL_BASE) + (LOCAL_TMR_CTRL_REG_OFFSET))
#define LOCAL_TMR_STAT_REG            ((ARMCTRL_BASE) + (LOCAL_TMR_STAT_REG_OFFSET))
#define LOCAL_TMR_CLR_REG             ((ARMCTRL_BASE) + (LOCAL_TMR_CLR_REG_OFFSET))
#define LOCAL_TMR_IRQ_RT              ((ARMCTRL_BASE) + (LOCAL_TMR_IRQ_RT_OFFSET))

// nCNTHPIRQ_IRQ
// nCNTPNSIRQ_IRQ

#define CORE0_TMR_IRQ_CNTL_REG_OFFSET 0x00000040
#define CORE0_TMR_IRQ_CNTL_REG        ((ARMCTRL_BASE) + (CORE0_TMR_IRQ_CNTL_REG_OFFSET))

extern caddr_t _bcm283x_iobase;          /* peripheral base address */
extern u32 _bcm283x_probably_qemu;    /* do we appear to be running on qemu? */

static u32 _use_coretimer_for_delay;
static u32 _use_coretimer_for_ticks;
static u32 _use_coretimer_for_intr;
static u32 _use_coretimer_for_freq;

static u32 _systimer_frequency;
static u32 _systimer_ticks_per_s;
static u32 _systimer_ticks_per_quantum;
static u32 _systimer_ticks_per_ms;
static u32 _systimer_ticks_per_us;

static u32 _intr_timer_frequency;
static u32 _intr_timer_ticks_per_s;
static u32 _intr_timer_ticks_per_quantum;
static u32 _intr_timer_ticks_per_ms;
static u32 _intr_timer_ticks_per_us;

extern u32 _get_cntfrq(void);
extern void _set_cntfrq(u32 v);
extern u64 _get_cntpct(void);
extern u32 _get_cntkctl(void);
extern void _set_cntkctl(u32 v);
extern u32 _get_cntp_tval(void);
extern void _set_cntp_tval(u32 v);
extern u32 _get_cntp_ctl(void);
extern void _set_cntp_ctl(u32 v);
extern u32 _get_cntv_tval(void);
extern void _set_cntv_tval(u32 v);
extern u32 _get_cntv_ctl(void);
extern void _set_cntv_ctl(u32 v);
extern u64 _get_cntvct(void);
extern u64 _get_cntp_cval(void);
extern void _set_cntp_cval(u64 v);
extern u64 _get_cntv_cval(void);
extern void _set_cntv_cval(u64 v);
extern u64 _get_cntvoff(void);
extern void _set_cntvoff(u64 v);
extern u32 _get_cnthctl(void);
extern void _set_cnthctl(u32 v);
extern u32 _get_cnthp_tval(void);
extern void _set_cnthp_tval(u32 v);
extern u32 _get_cnthp_ctl(void);
extern void _set_cnthp_ctl(u32 v);
extern u64 _get_cnthp_cval(void);
extern void _set_cnthp_cval(u64 v);

extern u32 read_cpsr(void);

extern void panic(const char *s) __attribute__((noreturn));

void bcm283x_systimer_early_init(void)
{
  switch (read_cpuid()) {
    case 0x410fb767:
      _use_coretimer_for_delay = 0;
      _use_coretimer_for_ticks = 0;
      _use_coretimer_for_intr = 0;
      _use_coretimer_for_freq = 0;
      break;
    default:
      if (_bcm283x_probably_qemu) {
        _use_coretimer_for_delay = 0;
        _use_coretimer_for_ticks = 0;
        _use_coretimer_for_intr = 1;
        _use_coretimer_for_freq = 1;
      } else {
        _use_coretimer_for_delay = 1;
        _use_coretimer_for_ticks = 1;
        _use_coretimer_for_intr = 1;
        _use_coretimer_for_freq = 1;
      }

      break;
  }

  if (_use_coretimer_for_freq || _use_coretimer_for_intr || _use_coretimer_for_ticks || _use_coretimer_for_delay) {
    /* do timer configuration */
    /* set up the core timer */
  }

  if (_use_coretimer_for_freq) {
    _systimer_frequency = _get_cntfrq();
    /* do initialisation work here - need to work out exactly what we need to do */
  } else {
    _systimer_frequency = 1000000;
  }

  if (_use_coretimer_for_intr) {
    iowrite32(CORE0_TMR_IRQ_CNTL_REG, 0xf);  /* route core0 CNTV core0 */
    // cntfrq = read_cntfrq();
    _intr_timer_frequency = _get_cntfrq();
    _intr_timer_ticks_per_s = _intr_timer_frequency;
    _intr_timer_ticks_per_quantum = _intr_timer_frequency / 10;
    _intr_timer_ticks_per_ms = _intr_timer_ticks_per_s / 1000;
    _intr_timer_ticks_per_us = _intr_timer_ticks_per_ms / 1000;

    // routing_core0cntv_to_core0irq();
    // enable_cntv();
    // enable_irq();
  }

  _systimer_ticks_per_s = _systimer_frequency;
  _systimer_ticks_per_quantum = _systimer_ticks_per_s / 10;
  _systimer_ticks_per_ms = _systimer_ticks_per_s / 1000;
  if (_systimer_ticks_per_ms * 1000 != _systimer_ticks_per_s) {
    _systimer_ticks_per_ms += 1;
  }

  _systimer_ticks_per_us = _systimer_ticks_per_ms / 1000;
  if (_systimer_ticks_per_us * 1000 != _systimer_ticks_per_ms) {
    _systimer_ticks_per_us += 1;
  }

  if (_use_coretimer_for_intr) {
    /* set up the core timer - leave interrupt stuff to clkinit (but rename this!) */
  }
}


void udelay(u32 us)
{
  if (_use_coretimer_for_delay) {
  } else {
    u32 now;
    now = ioread32(SYSTMR_CLO_REG);
    while (ioread32(SYSTMR_CLO_REG) - now < us);
  }
}


void mdelay(u32 ms)
{
  u32 m;

  for (m = 0; m < ms; ++m) {
    udelay(1000);
  }
}


u64 ticks(u64 *hz)
{
  if (_use_coretimer_for_ticks) {
    return 0;
  } else {
    u32 lo;
    u32 hi;

    if (hz) {
      *hz = (1000 * 1000);  /* 1Mhz */
    }

    do {
      hi = ioread32(SYSTMR_CHI_REG);
      lo = ioread32(SYSTMR_CLO_REG);
    } while (ioread32(SYSTMR_CHI_REG) != hi);

    return (((u64)hi) << 32) | lo;
  }
}


u32 tick_diff_us(u64 s, u64 e)
{
  if (e < s) {
    return ((0xffffffffffffffffULL - s) + e) / _systimer_ticks_per_us;
  }

  return (e - s) / _systimer_ticks_per_us;
}


extern void clock(int dev, u32 sp, u32 r1, int nps, u32 r0, u32 pc, u32 ps);


static void clkintr(void *arg, struct tf_regs_t *tf)
{
  if (_use_coretimer_for_intr) {
    _set_cntp_tval(_intr_timer_ticks_per_quantum);  /* clear cntv interrupt and set next 1 second timer */
  } else {
    iowrite32(SYSTMR_C1_REG, ioread32(SYSTMR_CLO_REG) + _systimer_ticks_per_quantum);  /* 1 second */
    iowrite32(SYSTMR_CS_REG, SYSTMR_CS_M1);
  }

  /* printf("clkintr: fired\n"); */
  clock(-1, tf->r13, tf->r1, read_cpsr(), tf->r0, tf->r15, tf->cpsr);
}


void clkinit()
{
  if (_use_coretimer_for_intr) {
    if (0 != bcm283x_register_timer_irq_handler(CORE_IRQ_CNTPSIRQ_INT_N(0), clkintr, NULL))
      panic("clkinit: failed to register IRQ handler");

    _set_cntp_tval(_intr_timer_ticks_per_quantum);  /* clear cntv interrupt and set next timer */
    _set_cntp_ctl(0x00000001);
  } else {
    if (0 != bcm283x_register_timer_irq_handler(GPU_IRQ_SYSTIMER_1_INT, clkintr, NULL))
      panic("clkinit: failed to register IRQ handler");

    iowrite32(SYSTMR_C1_REG, ioread32(SYSTMR_CLO_REG) - 1);
  }
}
