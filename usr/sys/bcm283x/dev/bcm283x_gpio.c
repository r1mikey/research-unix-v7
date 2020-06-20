#include "bcm283x_gpio.h"

#include "bcm283x_io.h"
#include "../arm1176jzfs.h"

#define GPIO_OFFSET                   0x00200000
#define GPIO_BASE                     ((_bcm283x_iobase) + (GPIO_OFFSET))

#define GPFSEL0_REG_OFFSET            0x00000000  /* GPIO Function Select 0                */
#define GPFSEL1_REG_OFFSET            0x00000004  /* GPIO Function Select 1                */
#define GPFSEL2_REG_OFFSET            0x00000008  /* GPIO Function Select 2                */
#define GPFSEL3_REG_OFFSET            0x0000000c  /* GPIO Function Select 3                */
#define GPFSEL4_REG_OFFSET            0x00000010  /* GPIO Function Select 4                */
#define GPFSEL5_REG_OFFSET            0x00000014  /* GPIO Function Select 5                */
#define GPSET0_REG_OFFSET             0x0000001c  /* GPIO Pin Output Set 0                 */
#define GPSET1_REG_OFFSET             0x00000020  /* GPIO Pin Output Set 1                 */
#define GPCLR0_REG_OFFSET             0x00000028  /* GPIO Pin Output Clear 0               */
#define GPCLR1_REG_OFFSET             0x0000002c  /* GPIO Pin Output Clear 1               */
#define GPLEV0_REG_OFFSET             0x00000034  /* GPIO Pin Level 0                      */
#define GPLEV1_REG_OFFSET             0x00000038  /* GPIO Pin Level 1                      */
#define GPEDS0_REG_OFFSET             0x00000040  /* GPIO Pin Event Detect Status 0        */
#define GPEDS1_REG_OFFSET             0x00000044  /* GPIO Pin Event Detect Status 1        */
#define GPREN0_REG_OFFSET             0x0000004c  /* GPIO Pin Rising Edge Detect Enable 0  */
#define GPREN1_REG_OFFSET             0x00000050  /* GPIO Pin Rising Edge Detect Enable 1  */
#define GPFEN0_REG_OFFSET             0x00000058  /* GPIO Pin Falling Edge Detect Enable 0 */
#define GPFEN1_REG_OFFSET             0x0000005c  /* GPIO Pin Falling Edge Detect Enable 1 */
#define GPHEN0_REG_OFFSET             0x00000064  /* GPIO Pin High Detect Enable 0         */
#define GPHEN1_REG_OFFSET             0x00000068  /* GPIO Pin High Detect Enable 1         */
#define GPLEN0_REG_OFFSET             0x00000070  /* GPIO Pin Low Detect Enable 0          */
#define GPLEN1_REG_OFFSET             0x00000074  /* GPIO Pin Low Detect Enable 1          */
#define GPAREN0_REG_OFFSET            0x0000007c  /* GPIO Pin Async. Rising Edge Detect 0  */
#define GPAREN1_REG_OFFSET            0x00000080  /* GPIO Pin Async. Rising Edge Detect 1  */
#define GPAFEN0_REG_OFFSET            0x00000088  /* GPIO Pin Async. Falling Edge Detect 0 */
#define GPAFEN1_REG_OFFSET            0x0000008c  /* GPIO Pin Async. Falling Edge Detect 1 */
#define GPPUD_REG_OFFSET              0x00000094  /* GPIO Pin Pull-up/down Enable          */
#define GPPUDCLK0_REG_OFFSET          0x00000098  /* GPIO Pin Pull-up/down Enable Clock 0  */
#define GPPUDCLK1_REG_OFFSET          0x0000009c  /* GPIO Pin Pull-up/down Enable Clock 1  */

#define GPFSEL0_REG                   ((GPIO_BASE) + (GPFSEL0_REG_OFFSET))
#define GPFSEL1_REG                   ((GPIO_BASE) + (GPFSEL1_REG_OFFSET))
#define GPFSEL2_REG                   ((GPIO_BASE) + (GPFSEL2_REG_OFFSET))
#define GPFSEL3_REG                   ((GPIO_BASE) + (GPFSEL3_REG_OFFSET))
#define GPFSEL4_REG                   ((GPIO_BASE) + (GPFSEL4_REG_OFFSET))
#define GPFSEL5_REG                   ((GPIO_BASE) + (GPFSEL5_REG_OFFSET))
#define GPSET0_REG                    ((GPIO_BASE) + (GPSET0_REG_OFFSET))
#define GPSET1_REG                    ((GPIO_BASE) + (GPSET1_REG_OFFSET))
#define GPCLR0_REG                    ((GPIO_BASE) + (GPCLR0_REG_OFFSET))
#define GPCLR1_REG                    ((GPIO_BASE) + (GPCLR1_REG_OFFSET))
#define GPLEV0_REG                    ((GPIO_BASE) + (GPLEV0_REG_OFFSET))
#define GPLEV1_REG                    ((GPIO_BASE) + (GPLEV1_REG_OFFSET))
#define GPEDS0_REG                    ((GPIO_BASE) + (GPEDS0_REG_OFFSET))
#define GPEDS1_REG                    ((GPIO_BASE) + (GPEDS1_REG_OFFSET))
#define GPREN0_REG                    ((GPIO_BASE) + (GPREN0_REG_OFFSET))
#define GPREN1_REG                    ((GPIO_BASE) + (GPREN1_REG_OFFSET))
#define GPFEN0_REG                    ((GPIO_BASE) + (GPFEN0_REG_OFFSET))
#define GPFEN1_REG                    ((GPIO_BASE) + (GPFEN1_REG_OFFSET))
#define GPHEN0_REG                    ((GPIO_BASE) + (GPHEN0_REG_OFFSET))
#define GPHEN1_REG                    ((GPIO_BASE) + (GPHEN1_REG_OFFSET))
#define GPLEN0_REG                    ((GPIO_BASE) + (GPLEN0_REG_OFFSET))
#define GPLEN1_REG                    ((GPIO_BASE) + (GPLEN1_REG_OFFSET))
#define GPAREN0_REG                   ((GPIO_BASE) + (GPAREN0_REG_OFFSET))
#define GPAREN1_REG                   ((GPIO_BASE) + (GPAREN1_REG_OFFSET))
#define GPAFEN0_REG                   ((GPIO_BASE) + (GPAFEN0_REG_OFFSET))
#define GPAFEN1_REG                   ((GPIO_BASE) + (GPAFEN1_REG_OFFSET))
#define GPPUD_REG                     ((GPIO_BASE) + (GPPUD_REG_OFFSET))
#define GPPUDCLK0_REG                 ((GPIO_BASE) + (GPPUDCLK0_REG_OFFSET))
#define GPPUDCLK1_REG                 ((GPIO_BASE) + (GPPUDCLK1_REG_OFFSET))

#define GPFUNC_INPUT                  0x00000000  /* GPIO Pin is an Input                  */
#define GPFUNC_OUTPUT                 0x00000001  /* GPIO Pin is an Input                  */
#define GPFUNC_ALT0                   0x00000004  /* GPIO Pin takes alternate function 0   */
#define GPFUNC_ALT1                   0x00000005  /* GPIO Pin takes alternate function 1   */
#define GPFUNC_ALT2                   0x00000006  /* GPIO Pin takes alternate function 2   */
#define GPFUNC_ALT3                   0x00000007  /* GPIO Pin takes alternate function 3   */
#define GPFUNC_ALT4                   0x00000003  /* GPIO Pin takes alternate function 4   */
#define GPFUNC_ALT5                   0x00000002  /* GPIO Pin takes alternate function 5   */
#define GPFUNC_MASK                   0x00000007  /* GPIO Function Mask                    */

extern devaddr_t _bcm283x_iobase;                 /* peripheral base address */


extern void denada(void);


void bcm283x_gpio_setup_for_pl011()
{
  uint32_t v;

  v = ioread32(GPFSEL1_REG);
  v &= ~((GPFUNC_MASK << 12) | (GPFUNC_MASK << 15));
  v |= ((GPFUNC_ALT0 << 12) | (GPFUNC_ALT0 << 15));
  iowrite32(GPFSEL1_REG, v);

  iowrite32(GPPUD_REG, 0);

  for (v = 0; v < 150; ++v) {
    denada();
  }

  iowrite32(GPPUDCLK0_REG, (1 << 14) | (1 << 15));

  for (v = 0; v < 150; ++v) {
    denada();
  }

  iowrite32(GPPUDCLK0_REG, 0);
}
