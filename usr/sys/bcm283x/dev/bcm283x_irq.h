#ifndef BCM283X_IRQ_H
#define BCM283X_IRQ_H

#include "../trap.h"
#include "../kstdint.h"


#define GPU_IRQ_BASE 0
#define GPU_IRQ_N(N) ((GPU_IRQ_BASE) + (N))

#define GPU_IRQ_00  0
#define GPU_IRQ_01  1
#define GPU_IRQ_02  2
#define GPU_IRQ_03  3
#define GPU_IRQ_04  4
#define GPU_IRQ_05  4
#define GPU_IRQ_06  6
#define GPU_IRQ_07  7
#define GPU_IRQ_08  8
#define GPU_IRQ_09  9
#define GPU_IRQ_10 10
#define GPU_IRQ_11 11
#define GPU_IRQ_12 12
#define GPU_IRQ_13 13
#define GPU_IRQ_14 14
#define GPU_IRQ_15 14
#define GPU_IRQ_16 16
#define GPU_IRQ_17 17
#define GPU_IRQ_18 18
#define GPU_IRQ_19 19
#define GPU_IRQ_20 20
#define GPU_IRQ_21 21
#define GPU_IRQ_22 22
#define GPU_IRQ_23 23
#define GPU_IRQ_24 24
#define GPU_IRQ_25 24
#define GPU_IRQ_26 26
#define GPU_IRQ_27 27
#define GPU_IRQ_28 28
#define GPU_IRQ_29 29
#define GPU_IRQ_30 30
#define GPU_IRQ_31 31
#define GPU_IRQ_32 32
#define GPU_IRQ_33 33
#define GPU_IRQ_34 34
#define GPU_IRQ_35 34
#define GPU_IRQ_36 36
#define GPU_IRQ_37 37
#define GPU_IRQ_38 38
#define GPU_IRQ_39 39
#define GPU_IRQ_40 40
#define GPU_IRQ_41 41
#define GPU_IRQ_42 42
#define GPU_IRQ_43 43
#define GPU_IRQ_44 44
#define GPU_IRQ_45 44
#define GPU_IRQ_46 46
#define GPU_IRQ_47 47
#define GPU_IRQ_48 48
#define GPU_IRQ_49 49
#define GPU_IRQ_50 50
#define GPU_IRQ_51 51
#define GPU_IRQ_52 52
#define GPU_IRQ_53 53
#define GPU_IRQ_54 54
#define GPU_IRQ_55 54
#define GPU_IRQ_56 56
#define GPU_IRQ_57 57
#define GPU_IRQ_58 58
#define GPU_IRQ_59 59
#define GPU_IRQ_60 60
#define GPU_IRQ_61 61
#define GPU_IRQ_62 62
#define GPU_IRQ_63 63
#define GPU_IRQ_MAX GPU_IRQ_63

#define GPU_IRQ_SYSTIMER_0_INT        GPU_IRQ_N( 0)
#define GPU_IRQ_SYSTIMER_1_INT        GPU_IRQ_N( 1)
#define GPU_IRQ_SYSTIMER_2_INT        GPU_IRQ_N( 2)
#define GPU_IRQ_SYSTIMER_3_INT        GPU_IRQ_N( 3)
#define GPU_IRQ_USB_INT               GPU_IRQ_N( 9)
#define GPU_IRQ_DMA_CHAN_00_INT       GPU_IRQ_N(16)
#define GPU_IRQ_DMA_CHAN_01_INT       GPU_IRQ_N(17)
#define GPU_IRQ_DMA_CHAN_02_INT       GPU_IRQ_N(18)
#define GPU_IRQ_DMA_CHAN_03_INT       GPU_IRQ_N(19)
#define GPU_IRQ_DMA_CHAN_04_INT       GPU_IRQ_N(20)
#define GPU_IRQ_DMA_CHAN_05_INT       GPU_IRQ_N(21)
#define GPU_IRQ_DMA_CHAN_06_INT       GPU_IRQ_N(22)
#define DMA_CHAN_IRQ(N)               ((GPU_IRQ_DMA_CHAN_00_INT) + (N))
#define GPU_IRQ_AUX_INT               GPU_IRQ_N(29)
#define GPU_IRQ_I2C_SPI_SLV_INT       GPU_IRQ_N(43)
#define GPU_IRQ_PWA0_INT              GPU_IRQ_N(45)
#define GPU_IRQ_PWA1_INT              GPU_IRQ_N(46)
#define GPU_IRQ_SMI_INT               GPU_IRQ_N(48)
#define GPU_IRQ_GPIO_INT0             GPU_IRQ_N(49)
#define GPU_IRQ_GPIO_INT1             GPU_IRQ_N(50)
#define GPU_IRQ_GPIO_INT2             GPU_IRQ_N(51)
#define GPU_IRQ_GPIO_INT3             GPU_IRQ_N(52)
#define GPU_IRQ_I2C_INT               GPU_IRQ_N(53)
#define GPU_IRQ_SPI_INT               GPU_IRQ_N(54)
#define GPU_IRQ_PCM_INT               GPU_IRQ_N(55)
#define GPU_IRQ_SDHOST_INT            GPU_IRQ_N(56)
#define GPU_IRQ_UART_INT              GPU_IRQ_N(57)
#define GPU_IRQ_EMMC_INT              GPU_IRQ_N(62)

#define ARM_IRQ_BASE ((GPU_IRQ_MAX) + (1))  /* 64 */
#define ARM_IRQ_N(N) ((ARM_IRQ_BASE) + (N))

/* BCM2835-ARM-Peripherals.pdf page 113 */
#define ARM_IRQ_ARM_TIMER             ARM_IRQ_N(0)  /* 64 */
#define ARM_IRQ_ARM_MAILBOX           ARM_IRQ_N(1)  /* 65 */
#define ARM_IRQ_ARM_DOORBELL_0        ARM_IRQ_N(2)  /* 66 */
#define ARM_IRQ_ARM_DOORBELL_1        ARM_IRQ_N(3)  /* 67 */
#define ARM_IRQ_GPU0_HALTED           ARM_IRQ_N(4)  /* 68 */
#define ARM_IRQ_GPU1_HALTED           ARM_IRQ_N(5)  /* 69 */
#define ARM_IRQ_ILLEGAL_ACCESS_TYPE_1 ARM_IRQ_N(6)  /* 70 */
#define ARM_IRQ_ILLEGAL_ACCESS_TYPE_2 ARM_IRQ_N(7)  /* 71 */

#define ARM_IRQ_MAX                   (ARM_IRQ_ILLEGAL_ACCESS_TYPE_2)  /* 71 */

#define PENDING_1_MASK                (1U << 8)
#define PENDING_2_MASK                (1U << 9)

#define CORE_IRQ_BASE ((ARM_IRQ_MAX) + 1)  /* 72 */
#define CORE_IRQ_N(N) ((CORE_IRQ_BASE) + (N))

#define CORE_IRQ_CNTPSIRQ_INT_N(N)    CORE_IRQ_N((12 * (N)) + 0)   /* 72 84  96 108 */
#define CORE_IRQ_CNTPNSIRQ_INT_N(N)   CORE_IRQ_N((12 * (N)) + 1)   /* 73 85  97 109 */
#define CORE_IRQ_CNTHPIRQ_INT_N(N)    CORE_IRQ_N((12 * (N)) + 2)   /* 74 86  98 110 */
#define CORE_IRQ_CNTVIRQ_INT_N(N)     CORE_IRQ_N((12 * (N)) + 3)   /* 75 87  99 111 */
#define CORE_IRQ_MAILBOX0_INT_N(N)    CORE_IRQ_N((12 * (N)) + 4)   /* 76 88 100 112 */
#define CORE_IRQ_MAILBOX1_INT_N(N)    CORE_IRQ_N((12 * (N)) + 5)   /* 77 89 101 113 */
#define CORE_IRQ_MAILBOX2_INT_N(N)    CORE_IRQ_N((12 * (N)) + 6)   /* 78 90 102 114 */
#define CORE_IRQ_MAILBOX3_INT_N(N)    CORE_IRQ_N((12 * (N)) + 7)   /* 79 91 103 115 */
#define CORE_IRQ_GPU_INT_N(N)         CORE_IRQ_N((12 * (N)) + 8)   /* 80 92 104 116 */
#define CORE_IRQ_PMU_INT_N(N)         CORE_IRQ_N((12 * (N)) + 9)   /* 81 93 105 117 */
#define CORE_IRQ_AXI_INT_N(N)         CORE_IRQ_N((12 * (N)) + 10)  /* 82 94 106 118 */
#define CORE_IRQ_LOCAL_TIMER_INT_N(N) CORE_IRQ_N((12 * (N)) + 11)  /* 83 95 107 119 */

#define CORE_IRQ_MAX                  (CORE_IRQ_LOCAL_TIMER_INT_N(3))


/*
 * This is an internal representation of an IRQ handler, declared
 * here so that other devices (like the AUX block) can cascade
 * handlers as needed without redeclaring this boilerplate.
 */
typedef struct irq_handler_t {
  void (*fn)(void*);
  void (*tfn)(void*, struct tf_regs_t *);
  void *arg;
} irq_handler_t;

extern void bcm283x_init_irq(void);
extern void bcm283x_deinit_irq(void);

extern int bcm283x_register_timer_irq_handler(uint32_t irqnum, void (*fn)(void*, struct tf_regs_t *), void *arg);
extern int bcm283x_register_irq_handler(uint32_t irqnum, void (*fn)(void*), void *arg);
extern int bcm283x_deregister_irq_handler(uint32_t irqnum);

#endif
